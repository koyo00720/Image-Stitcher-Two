#ifndef MAINCAMPUS_H
#define MAINCAMPUS_H

#include <QGraphicsView>
#include <QWidget>

class maincampus : public QGraphicsView
{
    Q_OBJECT
public:
    explicit maincampus(QWidget *parent = nullptr);
    void setCanvasBackground(bool white); // true=白, false=黒

signals:
    void zoomChanged(int percent);

protected:
    void wheelEvent(QWheelEvent *event) override;
    //void keyPressEvent(QKeyEvent *event) override;
};

#endif // MAINCAMPUS_H
