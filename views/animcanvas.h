#ifndef ANIMCANVAS_H
#define ANIMCANVAS_H

#include "canvasitem.h"

class Control;

class AnimCanvas : public QObject, public CanvasItem
{
    Q_OBJECT
public:
    AnimCanvas(QGraphicsItem * parent = nullptr);

public:
    QPixmap & snapshot()
    {
        return snapshot_;
    }

    void startAnimate(int dir);

    void updateAnimate();

    bool animate();

    void stopAnimate();

signals:
    void animateFinished();

private:
    QRectF animateRect(int dir, QPointF& off) const;

    void updateTransform();

    void setPos(QPointF const & pos);

private:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual void timerEvent(QTimerEvent *event) override;

private:
    QPixmap snapshot_;
    Control* canvasControl_;
    int animTimer_;
};

#endif // ANIMCANVAS_H
