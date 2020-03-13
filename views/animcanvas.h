#ifndef ANIMCANVAS_H
#define ANIMCANVAS_H

#include "canvasitem.h"

#include <QEasingCurve>

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

    void setScale(qreal scale);

private:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual void timerEvent(QTimerEvent *event) override;

private:
    QPixmap snapshot_;
    Control* canvasControl_;
    int timer_;
    int timeLine_;
    QPointF total_;
    QEasingCurve curve_;
};

#endif // ANIMCANVAS_H
