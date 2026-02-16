#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QString>
#include <QPixmap>
#include <QAction>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QtConcurrent/QtConcurrent>
#include <QIntValidator>

#include <QPointer>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <algorithm>
#include <cmath>

// QImageをOpenCV形式へ変換
static cv::Mat qimage_to_mat_bgra(const QImage& img)
{
    QImage converted = img.convertToFormat(QImage::Format_ARGB32); // 32-bit BGRA相当
    cv::Mat mat(converted.height(), converted.width(), CV_8UC4,
                (void*)converted.bits(), converted.bytesPerLine());
    return mat.clone(); // QImageの寿命から独立させる
}

// BGRAからAを取り出し、logical配列にする
cv::Mat1b alphaMaskFromBGRA(const cv::Mat& bgra, double alphaThreshold = 0.5)
{
    CV_Assert(!bgra.empty());
    CV_Assert(bgra.type() == CV_8UC4); // BGRA 8bit

    // alpha >= 0.5 → alpha >= 128
    const int thr = (int)std::lround(alphaThreshold * 255.0);

    std::vector<cv::Mat> ch;
    cv::split(bgra, ch);         // ch[3] が alpha
    cv::Mat1b mask;
    cv::compare(ch[3], thr, mask, cv::CMP_GE); // 0 or 255 のマスク

    // 「logical配列」(0/1)にしたいなら 0/255 を 0/1 に落とす
    mask /= 255;

    return mask; // CV_8U, 値は 0 or 1
}

// 最大矩形を探索する。
static inline void largestRectHistogram( // ヒストグラム最大矩形
    const std::vector<int>& h,
    int& bestArea, int& bestL, int& bestR, int& bestH)
{
    const int W = (int)h.size();
    bestArea = 0; bestL = 0; bestR = -1; bestH = 0;

    std::vector<int> st;
    st.reserve(W + 1);

    auto hh = [&](int i)->int { return (i == W) ? 0 : h[i]; };

    for (int i = 0; i <= W; ++i) {
        int cur = hh(i);
        while (!st.empty() && hh(st.back()) > cur) {
            int height = hh(st.back());
            st.pop_back();

            int left = st.empty() ? 0 : st.back() + 1;
            int right = i - 1;
            int area = height * (right - left + 1);

            if (area > bestArea) {
                bestArea = area;
                bestL = left;
                bestR = right;
                bestH = height;
            }
        }
        st.push_back(i);
    }
}

// 入力: logical配列 mask（CV_8U, 値0/1推奨。非0をtrue扱いでもOK）
// 出力: 最大面積矩形の (x,y,w,h)。無ければ (0,0,0,0)
cv::Rect maxRectOnesFromLogical(const cv::Mat1b& mask)
{
    CV_Assert(!mask.empty());
    CV_Assert(mask.channels() == 1);
    CV_Assert(mask.depth() == CV_8U);

    const int H = mask.rows;
    const int W = mask.cols;

    std::vector<int> heights(W, 0);

    int bestAreaAll = 0;
    int bestTop = 0, bestLeft = 0, bestBottom = -1, bestRight = -1;

    for (int r = 0; r < H; ++r) {
        const uchar* row = mask.ptr<uchar>(r);

        for (int c = 0; c < W; ++c) {
            if (row[c]) heights[c] += 1;
            else heights[c] = 0;
        }

        int area, l, rr, h;
        largestRectHistogram(heights, area, l, rr, h);

        if (area > bestAreaAll) {
            bestAreaAll = area;
            bestBottom = r;
            bestLeft = l;
            bestRight = rr;
            bestTop = r - h + 1;
        }
    }

    if (bestAreaAll <= 0) return cv::Rect(0, 0, 0, 0);

    return cv::Rect(
        bestLeft,
        bestTop,
        bestRight - bestLeft + 1,
        bestBottom - bestTop + 1
        );
}

// iFFT用関数
static cv::Mat1f clahe_then_grad(const cv::Mat& im_bgr)
{
    CV_Assert(!im_bgr.empty());
    CV_Assert(im_bgr.channels() == 3);

    cv::Mat g8;
    cv::cvtColor(im_bgr, g8, cv::COLOR_BGR2GRAY);

    // CLAHE
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(g8, g8);

    cv::Mat1f g;
    g8.convertTo(g, CV_32F);

    // Gaussian blur (sigma=1.0, ksize=(0,0) means auto)
    cv::GaussianBlur(g, g, cv::Size(0, 0), 1.0);

    // Sobel gradients
    cv::Mat1f gx, gy;
    cv::Sobel(g, gx, CV_32F, 1, 0, 3);
    cv::Sobel(g, gy, CV_32F, 0, 1, 3);

    // magnitude
    cv::Mat1f mag;
    cv::magnitude(gx, gy, mag);

    // mag -= mean; mag /= std
    cv::Scalar mean, stddev;
    cv::meanStdDev(mag, mean, stddev);
    mag -= (float)mean[0];
    float s = (float)stddev[0];
    if (s > 1e-6f) mag /= s;

    // Hanning window (reduce edge/DC effects)
    cv::Mat1f win;
    cv::createHanningWindow(win, mag.size(), CV_32F);
    mag = mag.mul(win);

    return mag;
}

static void paste_over(cv::Mat& dst, const cv::Mat& src, int x, int y)
{
    CV_Assert(dst.channels() == src.channels());
    CV_Assert(dst.depth() == src.depth());
    CV_Assert(x >= 0 && y >= 0);
    CV_Assert(x + src.cols <= dst.cols);
    CV_Assert(y + src.rows <= dst.rows);

    src.copyTo(dst(cv::Rect(x, y, src.cols, src.rows)));
}


// BGRA画像２枚を合成（距離変換フェザー）
// cam1, cam2: CV_8UC4 (BGRA)
// shift: phaseCorrelate(a,b) の戻り値を想定（あなたの符号規約に合わせて x2=-shift.x）
// featherRadius: フェザー幅（ピクセル）。0以下なら無制限（画像内側ほど重くなる）
cv::Mat make_canvas_bgra_feather_dt(
    const cv::Mat& cam1,
    const cv::Mat& cam2,
    const cv::Point2d& shift_from_phaseCorrelate,
    float featherRadius = 80.0f)
{
    CV_Assert(!cam1.empty() && !cam2.empty());
    CV_Assert(cam1.type() == CV_8UC4 && cam2.type() == CV_8UC4);

    // 貼り付けオフセット（あなたのコードと同じ）
    const int x1 = 0, y1 = 0;
    const int x2 = (int)std::lround(-shift_from_phaseCorrelate.x);
    const int y2 = (int)std::lround(-shift_from_phaseCorrelate.y);

    const int h1 = cam1.rows, w1 = cam1.cols;
    const int h2 = cam2.rows, w2 = cam2.cols;

    // キャンバスサイズ
    const int min_x = std::min(x1, x2);
    const int min_y = std::min(y1, y2);
    const int max_x = std::max(x1 + w1, x2 + w2);
    const int max_y = std::max(y1 + h1, y2 + h2);

    const int out_w = max_x - min_x;
    const int out_h = max_y - min_y;

    const int sx = -min_x;
    const int sy = -min_y;

    // 各画像をキャンバス座標に配置（未合成で保持）
    cv::Mat img1(out_h, out_w, CV_8UC4, cv::Scalar(0,0,0,0));
    cv::Mat img2(out_h, out_w, CV_8UC4, cv::Scalar(0,0,0,0));

    {
        cv::Rect roi1(x1 + sx, y1 + sy, w1, h1);
        CV_Assert(0 <= roi1.x && 0 <= roi1.y && roi1.x + roi1.width <= out_w && roi1.y + roi1.height <= out_h);
        cam1.copyTo(img1(roi1));
    }
    {
        cv::Rect roi2(x2 + sx, y2 + sy, w2, h2);
        CV_Assert(0 <= roi2.x && 0 <= roi2.y && roi2.x + roi2.width <= out_w && roi2.y + roi2.height <= out_h);
        cam2.copyTo(img2(roi2));
    }

    // 有効領域マスク（alpha > 0）
    cv::Mat1b m1(out_h, out_w, uchar(0));
    cv::Mat1b m2(out_h, out_w, uchar(0));

    for (int r = 0; r < out_h; ++r) {
        const cv::Vec4b* p1 = img1.ptr<cv::Vec4b>(r);
        const cv::Vec4b* p2 = img2.ptr<cv::Vec4b>(r);
        uchar* q1 = m1.ptr<uchar>(r);
        uchar* q2 = m2.ptr<uchar>(r);
        for (int c = 0; c < out_w; ++c) {
            q1[c] = (p1[c][3] > 0) ? 255 : 0;
            q2[c] = (p2[c][3] > 0) ? 255 : 0;
        }
    }

    // 距離変換（非ゼロ画素について、最も近いゼロ画素までの距離）
    // → 有効領域内部ほど距離が大きく、境界で0に近い
    cv::Mat1f d1, d2;
    cv::distanceTransform(m1, d1, cv::DIST_L2, 3);
    cv::distanceTransform(m2, d2, cv::DIST_L2, 3);

    // フェザー幅制御（任意）
    if (featherRadius > 0.0f) {
        cv::min(d1, featherRadius, d1);
        cv::min(d2, featherRadius, d2);
    }

    // 合成（フェザー）
    cv::Mat canvas(out_h, out_w, CV_8UC4, cv::Scalar(0,0,0,0));
    constexpr float eps = 1e-6f;

    for (int r = 0; r < out_h; ++r) {
        const cv::Vec4b* p1 = img1.ptr<cv::Vec4b>(r);
        const cv::Vec4b* p2 = img2.ptr<cv::Vec4b>(r);
        const float* dd1 = d1.ptr<float>(r);
        const float* dd2 = d2.ptr<float>(r);
        cv::Vec4b* out = canvas.ptr<cv::Vec4b>(r);

        for (int c = 0; c < out_w; ++c) {
            const cv::Vec4b a = p1[c];
            const cv::Vec4b b = p2[c];

            const float a1 = a[3] / 255.0f;
            const float a2 = b[3] / 255.0f;

            const bool v1 = a1 > 0.0f;
            const bool v2 = a2 > 0.0f;

            if (!v1 && !v2) {
                out[c] = cv::Vec4b(0,0,0,0);
                continue;
            }
            if (v1 && !v2) {
                out[c] = a;
                continue;
            }
            if (!v1 && v2) {
                out[c] = b;
                continue;
            }

            // 両方有効：距離から重み
            float ww1 = dd1[c];
            float ww2 = dd2[c];
            float wws = ww1 + ww2;

            // あり得る：境界ピッタリで両方ほぼ0 → その場合は等分
            if (wws < eps) { ww1 = 0.5f; ww2 = 0.5f;}
            else { ww1 /= wws; ww2 /= wws; }

            // αも含めて「事前乗算」で混ぜる（境界が破綻しにくい）
            // premult = rgb * alpha
            const float p1b = (a[0]/255.0f) * a1;
            const float p1g = (a[1]/255.0f) * a1;
            const float p1r = (a[2]/255.0f) * a1;

            const float p2b = (b[0]/255.0f) * a2;
            const float p2g = (b[1]/255.0f) * a2;
            const float p2r = (b[2]/255.0f) * a2;

            // フェザー重みで混合
            const float ao = std::clamp(a1*ww1 + a2*ww2, 0.0f, 1.0f); // 出力alpha
            float ob = 0.0f, og = 0.0f, or_ = 0.0f;

            if (ao > eps) {
                ob = (p1b*ww1 + p2b*ww2) / ao;
                og = (p1g*ww1 + p2g*ww2) / ao;
                or_ = (p1r*ww1 + p2r*ww2) / ao;
            }

            out[c][0] = (uchar)std::lround(std::clamp(ob, 0.0f, 1.0f) * 255.0f);
            out[c][1] = (uchar)std::lround(std::clamp(og, 0.0f, 1.0f) * 255.0f);
            out[c][2] = (uchar)std::lround(std::clamp(or_, 0.0f, 1.0f) * 255.0f);
            out[c][3] = (uchar)std::lround(ao * 255.0f);
        }
    }

    return canvas;
}

// 2つの画像から重なり領域をクロップして取り出す
return_struct2 Crop_2ImageTo2Image(cv::Mat input1, cv::Mat input2, QSize px1, QPoint pos1, QSize px2, QPoint pos2)
{
    // 座標移動ベクトルを計算
    int dx = std::min(pos1.x(), pos2.x());
    int dy = std::min(pos1.y(), pos2.y());

    // キャンパスサイズを計算
    int camX = std::max(pos1.x() + px1.width(), pos2.x() + px2.width()) - dx;
    int camY = std::max(pos1.y() + px1.height(), pos2.y() + px2.height()) - dy;

    // キャンパスを作成し、各画像を割り当て
    cv::Mat cam1(camY, camX, CV_8UC4, cv::Scalar(0,0,0,0));
    cv::Rect roi(pos1.x() - dx, pos1.y() - dy, input1.cols, input1.rows); // 貼り付け先座標
    input1.copyTo(cam1(roi));

    cv::Mat cam2(camY, camX, CV_8UC4, cv::Scalar(0,0,0,0));
    roi = cv::Rect(pos2.x() - dx, pos2.y() - dy, input2.cols, input2.rows); // 貼り付け先座標
    input2.copyTo(cam2(roi));

    // Alphaをlogical配列へ変換
    cv::Mat1b logicalMask1 = alphaMaskFromBGRA(cam1, 0.5); // 0/1
    cv::Mat1b logicalMask2 = alphaMaskFromBGRA(cam2, 0.5); // 0/1

    // 重なり領域を得る
    cv::Mat1b andMask;
    cv::bitwise_and(logicalMask1, logicalMask2, andMask);

    // and領域を矩形化する
    cv::Rect rect = maxRectOnesFromLogical(andMask);

    // 画像を3ch化
    cv::Mat cam1_3ch, cam2_3ch;
    cv::cvtColor(cam1, cam1_3ch, cv::COLOR_BGRA2BGR);
    cv::cvtColor(cam2, cam2_3ch, cv::COLOR_BGRA2BGR);

    // 重なり領域をcropして取り出す
    cv::Mat crop1 = cam1_3ch(rect).clone();
    cv::Mat crop2 = cam2_3ch(rect).clone();

    // 返り値を設定
    return_struct2 r;
    r.img1 = crop1;
    r.img2 = crop2;
    return r;
};

// SSIM計算関数
static double ssim_single_channel(const cv::Mat& i1u8, const cv::Mat& i2u8)
{
    cv::Mat I1, I2;
    i1u8.convertTo(I1, CV_32F);
    i2u8.convertTo(I2, CV_32F);

    const double C1 = (0.01 * 255) * (0.01 * 255);
    const double C2 = (0.03 * 255) * (0.03 * 255);

    cv::Mat mu1, mu2;
    cv::GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);

    cv::Mat mu1_2 = mu1.mul(mu1);
    cv::Mat mu2_2 = mu2.mul(mu2);
    cv::Mat mu1_mu2 = mu1.mul(mu2);

    cv::Mat sigma1_2, sigma2_2, sigma12;
    cv::GaussianBlur(I1.mul(I1), sigma1_2, cv::Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;

    cv::GaussianBlur(I2.mul(I2), sigma2_2, cv::Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;

    cv::GaussianBlur(I1.mul(I2), sigma12, cv::Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    cv::Mat t1 = 2 * mu1_mu2 + C1;
    cv::Mat t2 = 2 * sigma12 + C2;
    cv::Mat t3 = mu1_2 + mu2_2 + C1;
    cv::Mat t4 = sigma1_2 + sigma2_2 + C2;

    cv::Mat ssim_map = (t1.mul(t2)) / (t3.mul(t4));
    return cv::mean(ssim_map)[0];
}

double ssim(const cv::Mat& a, const cv::Mat& b)
{
    CV_Assert(!a.empty() && !b.empty());
    CV_Assert(a.size() == b.size());

    cv::Mat A = a, B = b;

    // SSIMは基本「同じチャンネル数」で。迷ったらグレースケールに落とすのが簡単
    if (A.channels() == 3) cv::cvtColor(A, A, cv::COLOR_BGR2GRAY);
    if (B.channels() == 3) cv::cvtColor(B, B, cv::COLOR_BGR2GRAY);
    if (A.channels() == 4) cv::cvtColor(A, A, cv::COLOR_BGRA2GRAY);
    if (B.channels() == 4) cv::cvtColor(B, B, cv::COLOR_BGRA2GRAY);

    CV_Assert(A.type() == CV_8U && B.type() == CV_8U);
    return ssim_single_channel(A, B);
}

// SSIM 各スレッドの計算処理
static double SSIM_calc_oneshot(const SSIM_TaskInput& in)
{
    // 2枚目画像を(dx,dy)移動
    QPoint in_pos2(in.pos2.x() + in.dx, in.pos2.y() + in.dy);

    // 重なり領域をcropして取り出す。
    return_struct2 r_st = Crop_2ImageTo2Image(in.input1, in.input2, in.px1, in.pos1, in.px2, in_pos2);
    cv::Mat crop1 = r_st.img1;
    cv::Mat crop2 = r_st.img2;

    /*
    cv::imshow("crop1",crop1);
    cv::imshow("crop2",crop2);
    cv::waitKey(0);
    */

    if (crop1.rows == 0) {
        return 0.0;
    }

    double v = ssim(crop1, crop2);
    return v;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton_1, &QPushButton::clicked, this, &MainWindow::Image_in_clicked);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::Front_Back);
    connect(ui->graphicsView, &maincampus::zoomChanged,
            this, [this](int pct){ zoomLabel->setText(QString("%1%").arg(pct)); });

    connect(ui->dropArea, &DropArea::filesDropped, this, [this](const QStringList& paths){
        File_input(paths);
    });

    scene = new QGraphicsScene(this);

    ui->graphicsView->setScene(scene);

    // imageの削除
    auto *actDelete = new QAction(this);
    actDelete->setShortcut(QKeySequence::Delete);
    actDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut); // MainWindow配下で有効
    addAction(actDelete);
    connect(actDelete, &QAction::triggered, this, &MainWindow::deleteSelectedItems);

    // 拡大率表示
    zoomLabel = new QLabel(this);
    zoomLabel->setText("100%");
    statusBar()->addPermanentWidget(zoomLabel);

    // 透明度制御
    ui->sliderOpacity1->setRange(0, 100);
    ui->spinOpacity1->setRange(0, 100);
    ui->sliderOpacity1->setValue(0);
    ui->spinOpacity1->setValue(0);

    ui->sliderOpacity2->setRange(0, 100);
    ui->spinOpacity2->setRange(0, 100);
    ui->sliderOpacity2->setValue(0);
    ui->spinOpacity2->setValue(0);

    // --- 同期：slider <-> spin（画像1）---
    connect(ui->sliderOpacity1, &QSlider::valueChanged,
            ui->spinOpacity1, &QSpinBox::setValue);
    connect(ui->spinOpacity1, QOverload<int>::of(&QSpinBox::valueChanged),
            ui->sliderOpacity1, &QSlider::setValue);

    // 透明度反映（画像1）
    connect(ui->sliderOpacity1, &QSlider::valueChanged,
            this, &MainWindow::onOpacity1Changed);

    // --- 同期：slider <-> spin（画像2）---
    connect(ui->sliderOpacity2, &QSlider::valueChanged,
            ui->spinOpacity2, &QSpinBox::setValue);
    connect(ui->spinOpacity2, QOverload<int>::of(&QSpinBox::valueChanged),
            ui->sliderOpacity2, &QSlider::setValue);

    // 透明度反映（画像2）
    connect(ui->sliderOpacity2, &QSlider::valueChanged,
            this, &MainWindow::onOpacity2Changed);

    // 背景色を設定
    ui->comboBox->clear();
    ui->comboBox->addItems({"Black", "White"});

    auto applyBg = [this](int index) {
        // 0: White, 1: Black
        ui->graphicsView->setBackgroundBrush(index == 0 ? Qt::black : Qt::white);
    };

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, applyBg);

    applyBg(ui->comboBox->currentIndex());

    // 計算開始ボタン
    connect(ui->pushButton_Calc1, &QPushButton::clicked, this, &MainWindow::calc_iFFT);

    // 計算完了通知を受け取る
    connect(&m_ifftWatcher, &QFutureWatcher<cv::Mat>::finished, this, &MainWindow::iFFT_finish);

    // 結合ボタン
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::stitch_image12);

    // PNG exportボタン
    connect(ui->pushButton_4, &QPushButton::clicked, this, &MainWindow::png_export);

    // 計算開始ボタン
    connect(ui->pushButton_Calc2, &QPushButton::clicked, this, &MainWindow::calc_SSIM);

    // 計算完了通知を受け取る
    connect(&m_ssimWatcher, &QFutureWatcher<return_struct1>::finished,
            this, &MainWindow::ssim_finish,
            Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

static int viewZoomPercent(const QGraphicsView *view)
{
    const double s = view->transform().m11();   // x方向スケール（等方ならこれでOK）
    return int(std::round(s * 100.0));
}

static double percentToOpacity(int percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    return 1.0 - (percent / 100.0); // 0.0〜1.0
}

void MainWindow::Image_in_clicked()
{
    const QStringList paths = QFileDialog::getOpenFileNames(
        this, "Open Image(s)", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;All Files (*.*)"
        ); // ファイル取得ダイアログを作成し、string配列としてファイルパスを取得。

    if (paths.isEmpty()) // キャンセルが押された場合
    {
         return;
    }

    //File_Path_data = paths;
    File_input(paths);
}

void MainWindow::File_input(QStringList paths) {
    int const n = paths.size();
    for (int i = 0; i < n; ++i) {

        // 画像ファイルとして読み込めるか確認
        QPixmap pix(paths[i]);
        if (pix.isNull()) {
            QMessageBox::warning(this, "error", QString("%1枚目の画像の読み込みに失敗しました。").arg(i + 1));
            continue; // 次のiへ進む
        }

        QGraphicsPixmapItem **target = nullptr;
        if (item1 == nullptr) {
            target = &item1;
            exp_png1 = paths[i];
        } else if (item2 == nullptr) {
            target = &item2;
            exp_png2 = paths[i];
        } else {
            QMessageBox::warning(this, "error", "入力できる画像は2枚までです。");
            return; // forを終了
        }

        // z値を計算
        z_value++;

        if (*target == nullptr) {
            *target = scene->addPixmap(pix);
            (*target)->setFlags(QGraphicsItem::ItemIsMovable |
                                QGraphicsItem::ItemIsSelectable |
                                QGraphicsItem::ItemIsFocusable);
        } else {
            (*target)->setPixmap(pix);
        }
        (*target)->setZValue(z_value);
    }

    if (item2 != nullptr) {
        ui->sliderOpacity2->setValue(40);
    }
}

void MainWindow::deleteSelectedItems()
{
    const auto selected = scene->selectedItems();
    if (selected.isEmpty()) return;

    for (QGraphicsItem *it : selected) {
        // item1/item2 なら nullptr に戻す
        if (it == item1) {
            item1 = item2;
            exp_png1 = exp_png2;
        }

        ui->sliderOpacity1->setValue(0);
        ui->sliderOpacity2->setValue(0);
        onOpacity1Changed(0);
        onOpacity2Changed(0);

        item2 = nullptr;
        scene->removeItem(it);
        delete it;
    }
}


void MainWindow::Front_Back()
{
    if (!item1 || !item2) {
        QMessageBox::warning(this, "error", "画像を2枚inputしてください。");
        return; // どちらか未定義の場合
    }

    const qreal z1 = item1->zValue();
    const qreal z2 = item2->zValue();

    item1->setZValue(z2);
    item2->setZValue(z1);
}


void MainWindow::updateZoomLabel()
{
    const int pct = viewZoomPercent(ui->graphicsView);
    zoomLabel->setText(QString("%1%").arg(pct));
}

void MainWindow::setOpacityForItem(QGraphicsPixmapItem *item, int percent)
{
    if (!item) return; // まだ画像が無い
    item->setOpacity(percentToOpacity(percent));
}

void MainWindow::onOpacity1Changed(int percent)
{
    setOpacityForItem(item1, percent);
}

void MainWindow::onOpacity2Changed(int percent)
{
    setOpacityForItem(item2, percent);
}

// iFFTを別スレッドで開始
void MainWindow::calc_iFFT()
{
    // 画像があるか判定
    if (!item1 || item1->pixmap().isNull() ||
        !item2 || item2->pixmap().isNull()) {
        QMessageBox::warning(this, "OpenCV", "Calc. need two images.");
        return;
    }

    // 画像データをOpenCV向けに変換
    cv::Mat mat1, mat2;
    QImage img_QI = item1->pixmap().toImage();
    mat1 = qimage_to_mat_bgra(img_QI);   // CV_8UC4 (BGRA)
    img_QI = item2->pixmap().toImage();
    mat2 = qimage_to_mat_bgra(img_QI);

    // 入力をコピーしておく（別スレッドで安全に使うため）
    cv::Mat input1 = mat1.clone();
    cv::Mat input2 = mat2.clone();

    // その他の入力値を取得
    QSize px1 = item1->pixmap().size();
    QPointF pos1f = item1->pos();
    QSize px2 = item2->pixmap().size();
    QPointF pos2f = item2->pos();

    // 位置を負の無限大方向へ丸め
    QPoint pos1(
        static_cast<int>(std::floor(pos1f.x())),
        static_cast<int>(std::floor(pos1f.y()))
        );
    QPoint pos2(
        static_cast<int>(std::floor(pos2f.x())),
        static_cast<int>(std::floor(pos2f.y()))
        );

    // QtConcurrentで別スレッド実行
    auto future = QtConcurrent::run([input1, input2, px1, pos1, px2, pos2]() -> return_struct1 {
        // ここは別スレッド。UI触らない。

        // 重なり領域をcropして取り出す。
        return_struct2 r_st = Crop_2ImageTo2Image(input1, input2, px1, pos1, px2, pos2);
        cv::Mat crop1 = r_st.img1;
        cv::Mat crop2 = r_st.img2;

        /*
        cv::imshow("crop1",crop1);
        cv::imshow("crop2",crop2);
        cv::waitKey(0);
        */

        if (crop1.rows == 0) {
            return_struct1 r;
            r.score = 0;
            r.x = 0;
            r.y = 0;
            return r;
        }

        // 以下、計算
        cv::Mat1f a = clahe_then_grad(crop1);
        cv::Mat1f b = clahe_then_grad(crop2);

        // 位相相関法による位置合わせ
        double response = 0.0;
        cv::Point2d shift = cv::phaseCorrelate(a, b, cv::noArray(), &response);

        // 四捨五入
        cv::Point2d shift_r(std::round(shift.x), std::round(shift.y));

        // 2枚目画像の位置計算
        int x_r = pos2.x() - pos1.x() - shift_r.x;
        int y_r = pos2.y() - pos1.y() - shift_r.y;

        return_struct1 r;
        r.score = response;
        r.x = x_r;
        r.y = y_r;

        return r;
    });

    m_ifftWatcher.setFuture(future);

    // 例: UIを一時無効化など（任意）
    // ui->btnStart->setEnabled(false);

}

void MainWindow::iFFT_finish()
{
    return_struct1 result = m_ifftWatcher.result();
    ui->label_5->setText(QString::number(result.score));

    if (result.score != 0) {
        item1->setPos(0, 0);
        item2->setPos(result.x, result.y);

        // SSIM計算
        // 画像データをOpenCV向けに変換
        cv::Mat mat1, mat2;
        QImage img_QI = item1->pixmap().toImage();
        mat1 = qimage_to_mat_bgra(img_QI);   // CV_8UC4 (BGRA)
        img_QI = item2->pixmap().toImage();
        mat2 = qimage_to_mat_bgra(img_QI);

        // 入力をコピーしておく（別スレッドで安全に使うため）
        cv::Mat input1 = mat1.clone();
        cv::Mat input2 = mat2.clone();

        // その他の入力値を取得
        QSize px1 = item1->pixmap().size();
        QPointF pos1f = item1->pos();
        QSize px2 = item2->pixmap().size();
        QPointF pos2f = item2->pos();

        // 位置を負の無限大方向へ丸め
        QPoint pos1(
            static_cast<int>(std::floor(pos1f.x())),
            static_cast<int>(std::floor(pos1f.y()))
            );
        QPoint pos2(
            static_cast<int>(std::floor(pos2f.x())),
            static_cast<int>(std::floor(pos2f.y()))
            );


        SSIM_TaskInput ssim_input_one{input1,input2,px1,pos1,px2,pos2,0,0};

        double score_now = SSIM_calc_oneshot(ssim_input_one);
        ui->label_7->setText(QString::number(score_now));

    } else {
        QMessageBox::warning(this, "Calc. iFFT", "画像間の重なりが見つけられませんでした。");
    }
}

void MainWindow::stitch_image12() {
    if (item1 == nullptr && item2 == nullptr) {
        QMessageBox::warning(this, "PNG export", "結合する画像がありません。");
        return;
    } else if (item1 == nullptr || item2 == nullptr) {
        QMessageBox::warning(this, "PNG export", "結合には画像が2枚必要です。");
        return;
    }

    // OpenCV形式へ画像を変換
    cv::Mat mat1, mat2;
    QImage img_QI = item1->pixmap().toImage();
    mat1 = qimage_to_mat_bgra(img_QI);
    img_QI = item2->pixmap().toImage();
    mat2 = qimage_to_mat_bgra(img_QI);

    QPointF pos1f = item1->pos();
    QPointF pos2f = item2->pos();

    // 位置を負の無限大方向へ丸め
    QPoint pos1(
        static_cast<int>(std::floor(pos1f.x())),
        static_cast<int>(std::floor(pos1f.y()))
        );
    QPoint pos2(
        static_cast<int>(std::floor(pos2f.x())),
        static_cast<int>(std::floor(pos2f.y()))
        );

    cv::Point2d shiftV(pos1.x() - pos2.x(), pos1.y() - pos2.y());

    cv::Mat output = make_canvas_bgra_feather_dt(mat1, mat2, shiftV, /*featherRadius=*/80.0f);
    //cv::imshow("test", output);
    //cv::waitKey(0);

    // item1へ結合画像を代入
    QImage qimg(output.data, output.cols, output.rows, output.step, QImage::Format_ARGB32);
    QPixmap px = QPixmap::fromImage(qimg.copy());
    item1->setPixmap(px);
    item1->setPos(0, 0);

    // item2を初期化
    delete item2;
    item2 = nullptr;

    // 透明度を初期化
    ui->sliderOpacity1->setValue(0);
    onOpacity1Changed(0);
    ui->sliderOpacity2->setValue(0);
}

void MainWindow::png_export() {

    if (item1 == nullptr && item2 == nullptr) {
        QMessageBox::warning(this, "PNG export", "出力できる画像がありません。");
        return;
    } else if (item1 == nullptr && item2 != nullptr) {
        QMessageBox::warning(this, "PNG export", "何かがおかしいようです。");
        return;
    } else if (item1 != nullptr && item2 != nullptr) {
        QMessageBox::warning(this, "PNG export", "先に画像を結合してください。");
        return;
    }

    QFileInfo fi(exp_png1);
    QDir dir = fi.dir();

    QString newName = "stitched_" + fi.completeBaseName() + ".png";
    QString initialPath = dir.filePath(newName);

    QString newpath = QFileDialog::getSaveFileName(
        this,
        "Save File",
        initialPath,
        "PNG Image (*.png);;All Files (*.*)"
        );

    if (newpath.isEmpty()) // キャンセルが押された場合
    {
        return;
    }

    QImage img = item1->pixmap().toImage();   // RGBA保持
    img.save(newpath, "PNG");
}

// SSIM 各スレッドのデータ構造化
static return_struct1 SSIM_calc_oneshot_struct(const SSIM_TaskInput& in)
{
    return_struct1 r;
    r.x = in.pos2.x() - in.pos1.x() + in.dx;
    r.y = in.pos2.y() - in.pos1.y() + in.dy;
    r.score = SSIM_calc_oneshot(in); // double を返す純計算
    return r;
}

// スコア最大だけ保持
static void SSIM_calc_reduceMax(return_struct1& acc, const return_struct1& v)
{
    if (v.score > acc.score) acc = v;
}

void MainWindow::calc_SSIM() {
    if (m_ssimWatcher.isRunning()) return; // 連打防止

    int i_pix = ui->spinBoxSSIM->value();

    // 画像があるか判定
    if (!item1 || item1->pixmap().isNull() ||
        !item2 || item2->pixmap().isNull()) {
        QMessageBox::warning(this, "OpenCV", "Calc. need two images.");
        return;
    }

    // 画像データをOpenCV向けに変換
    QImage img1 = item1->pixmap().toImage();
    QImage img2 = item2->pixmap().toImage();

    cv::Mat mat1 = qimage_to_mat_bgra(img1); // CV_8UC4
    cv::Mat mat2 = qimage_to_mat_bgra(img2);

    // 入力をコピーしておく（別スレッドで安全に使うため）
    cv::Mat input1 = mat1.clone();
    cv::Mat input2 = mat2.clone();

    // その他の入力値を取得
    QSize px1 = item1->pixmap().size();
    QPointF pos1f = item1->pos();
    QSize px2 = item2->pixmap().size();
    QPointF pos2f = item2->pos();

    // 位置を負の無限大方向へ丸め
    QPoint pos1(
        static_cast<int>(std::floor(pos1f.x())),
        static_cast<int>(std::floor(pos1f.y()))
        );
    QPoint pos2(
        static_cast<int>(std::floor(pos2f.x())),
        static_cast<int>(std::floor(pos2f.y()))
        );

    // 入力変数群を用意
    const int N = (2 * i_pix + 1) * (2 * i_pix + 1);
    QVector<SSIM_TaskInput> inputs;
    inputs.reserve(N);

    for (int ix = -i_pix; ix <= i_pix; ++ix) {
        for (int iy = -i_pix; iy <= i_pix; ++iy) {
            inputs.push_back(SSIM_TaskInput{input1, input2, px1, pos1, px2, pos2, ix, iy});
        }
    }

    ssim_inputs_save = inputs;

    // 集約まで含めて並列実行（戻り値は return_struct1 1個）
    QFuture<return_struct1> future = QtConcurrent::mappedReduced(
        inputs,
        SSIM_calc_oneshot_struct,
        SSIM_calc_reduceMax,
        QtConcurrent::UnorderedReduce  // 順序不要ならこれが速いことが多い
        );

    // watcherへ
    m_ssimWatcher.setFuture(future);

    // UIは開始時にロック（任意）
    //ui->runButton->setEnabled(false);

}

void MainWindow::ssim_finish()
{
    return_struct1 result = m_ssimWatcher.future().result();

    ui->label_7->setText(QString::number(result.score));

    if (result.score != 0) {
        item1->setPos(0, 0);
        item2->setPos(result.x, result.y);
        ui->label_5->setText(QString("-"));
    } else {
        QMessageBox::warning(this, "Calc. SSIM", "画像間の重なりが見つけられませんでした。");
    }
}
