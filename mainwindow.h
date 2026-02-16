#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLabel>
#include <QFutureWatcher>

#include <opencv2/core.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QLabel;

struct return_struct1 {
    double score = 0.0;
    int x = 0;
    int y = 0;
};

struct return_struct2 {
    cv::Mat img1;
    cv::Mat img2;
};

// SSIM 1スレッドの入力
struct SSIM_TaskInput {
    cv::Mat input1;
    cv::Mat input2;
    QSize px1;
    QPoint pos1;
    QSize px2;
    QPoint pos2;
    int dx;
    int dy;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void Image_in_clicked();
    void Front_Back();
    void onOpacity1Changed(int percent);
    void onOpacity2Changed(int percent);
    void calc_iFFT(); // ボタンを押した時に実行
    void iFFT_finish(); // 計算完了時に実行
    void stitch_image12(); // 結合ボタンを押した時に実行
    void png_export(); // exportボタンを押した時に実行
    void calc_SSIM(); // ボタンを押した時に実行
    void ssim_finish(); // 計算完了時に実行

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    // z値
    int z_value = 1;

    // ファイルインプット
    void File_input(QStringList);

    // inputファイル名
    QString exp_png1;
    QString exp_png2;

    // 画像データの保持
    QGraphicsPixmapItem *item1 = nullptr;
    QGraphicsPixmapItem *item2 = nullptr;

    // 画像データの削除
    void deleteSelectedItems();

    // 拡大率表示
    QLabel *zoomLabel = nullptr;
    void updateZoomLabel();

    // 透明度制御
    void setOpacityForItem(QGraphicsPixmapItem *item, int percent);

    // iFFTの戻り値
    QFutureWatcher<return_struct1> m_ifftWatcher;

    // SSIMの戻り値
    QFutureWatcher<return_struct1> m_ssimWatcher;

    // SSIM入力値を保存
    QVector<SSIM_TaskInput> ssim_inputs_save;
};

class MyGraphicsView : public QGraphicsView
{
protected:
    void wheelEvent(QWheelEvent *event) override;
};


#endif // MAINWINDOW_H
