#include "maincampus.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QBrush>
#include <QColor>
#include <cmath>

maincampus::maincampus(QWidget *parent) : QGraphicsView(parent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    //setFocusPolicy(Qt::StrongFocus);
}


static int zoomPercent(const QGraphicsView *v)
{
    return int(std::round(v->transform().m11() * 100.0));
}


void maincampus::wheelEvent(QWheelEvent *event)
{
    const auto mods = event->modifiers();

    // Ctrl + ホイール：ズーム
    if (mods & Qt::ControlModifier) {
        const double zoomFactor = 1.1;
        if (event->angleDelta().y() > 0)
            scale(zoomFactor, zoomFactor);
        else
            scale(1.0 / zoomFactor, 1.0 / zoomFactor);

        event->accept();

        emit zoomChanged(zoomPercent(this)); // 拡大率表示
        return;
    }

    // Shift + ホイール：横スクロール（現状維持）
    if (mods & Qt::ShiftModifier) {
        int delta = event->angleDelta().x();
        if (delta == 0) delta = event->angleDelta().y();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta);
        event->accept();
        return;
    }

    // 何も押してない：通常の縦スクロール（QGraphicsView標準動作）
    QGraphicsView::wheelEvent(event);
}

// 背景色を設定
void maincampus::setCanvasBackground(bool white)
{
    setBackgroundBrush(white ? QBrush(Qt::white) : QBrush(Qt::black));
}
