#ifndef ANIMCANVAS_H
#define ANIMCANVAS_H

#include "canvasitem.h"

#include <QEasingCurve>

class Control;

class AnimCanvas : public QObject, public CanvasItem
{
    Q_OBJECT
public:
    enum AnimateDirection
    {
        LeftToRight = 1,
        RightToLeft = 2,
        TopToBottom = 4,
        BottomToTop = 8,
        LeftTopToRight = 5,
        RightBottomToLeftTop = 10,
        LeftBottomToRightTop = 9,
        RightTopToLeftBottomTop = 6,
    };

    Q_ENUM(AnimateDirection)

    AnimCanvas(QGraphicsItem * parent = nullptr);

    virtual ~AnimCanvas() override;

public:
    void setSnapshot(QPixmap ss);

    AnimateDirection direction() const { return direction_; }

    void setDirection(AnimateDirection dir);

    bool afterPageSwitch() const { return afterPageSwitch_; }

    //  true: if canvas is holding new page
    //  false: if canvas is holding old page, switch to new page after animation
    void setAfterPageSwitch(bool after = true);

    bool switchPage() const { return switchPage_; }

    bool inAnimate() const;

    void startAnimate();

    void updateCanvas();

    // return true if reverted
    bool move(QPointF const & offset);

    bool release();

    void stopAnimate();

signals:
    void animateFinished(bool finished);

private:
    void initAnimate();

    void termAnimate();

    void initTranform();

    void updateTransform();

    bool animate();

    void setRect(QRectF const & rect, QPointF const & off);

    void setPos(QPointF const & pos);

    void setScale(qreal scale);

    QRectF animateRect(QPointF& off) const;

private:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual void timerEvent(QTimerEvent *event) override;

private:
    QPixmap snapshot_;
    Control* canvasControl_;
    int timer_;
    int timeLine_;
    AnimateDirection direction_;
    bool afterPageSwitch_;
    bool switchPage_;
    QPointF total_;
    QEasingCurve curve_;
};

#endif // ANIMCANVAS_H
