#include "animcanvas.h"
#include "pagecanvas.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QDebug>

AnimCanvas::AnimCanvas(QGraphicsItem * parent)
    : CanvasItem(parent)
    , canvasControl_(nullptr)
    , timer_(0)
    , timeLine_(0)
    , curve_(QEasingCurve::InOutQuart)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

void AnimCanvas::startAnimate(int dir)
{
    if (timer_)
        return;
    setBrush(scene()->backgroundBrush());
    scene()->setBackgroundBrush(QBrush());
    updateAnimate();
    QPointF off;
    QRectF r = animateRect(dir, off);
    setRect(r);
    setTransformOriginPoint(r.center());
    qDebug() << "AnimCanvas::startAnimate" << dir << rect();
    PageCanvas * canvas = static_cast<PageCanvas*>(parentItem()->childItems().first());
    r.translate(-off);
    canvas->setRect(r);
    canvas->setTransformOriginPoint(r.center());
    canvas->setBrush(brush());
    canvas->setFlag(ItemHasNoContents, false);
    canvas->setFlag(ItemClipsChildrenToShape, true);
    total_ = -off;
    setPos(total_);
    timer_ = startTimer(20);
}

void AnimCanvas::updateAnimate()
{
    canvasControl_ = Control::fromItem(parentItem());
    if (canvasControl_) {
        canvasControl_->metaObject()->invokeMethod(
                    canvasControl_, "setPosBarVisible", Q_ARG(bool,false));
        connect(&canvasControl_->resource()->transform(),
                         &ResourceTransform::changed, this, [this] () {
            updateTransform();
        });
    }
    updateTransform();
}

void AnimCanvas::updateTransform()
{
    if (!timer_)
        return;
    int dir = 0;
    if (!qFuzzyIsNull(total_.x()))
        dir |= total_.x() < 0 ? 1 : 2;
    if (!qFuzzyIsNull(total_.y()))
        dir |= total_.y() < 0 ? 4 : 8;
    QRectF old = rect();
    QPointF off;
    QRectF r = animateRect(dir, off);
    setRect(r);
    setTransformOriginPoint(r.center());
    qDebug() << "AnimCanvas::updateAnimate" << dir << rect();
    QPointF pos = this->pos();
    pos *= r.width() / old.width();
    total_ *= r.width() / old.width();
    setPos(pos);
    update();
    PageCanvas * canvas = static_cast<PageCanvas*>(parentItem()->childItems().first());
    r.translate(-off);
    canvas->setRect(r);
    canvas->setTransformOriginPoint(r.center());
    canvas->update();
}

bool AnimCanvas::animate()
{
    ++timeLine_;
    if (timeLine_ <= 5) {
        setScale(1 - 0.02 * timeLine_);
    } else if (timeLine_ <= 25) {
        qreal v = curve_.valueForProgress((25 - timeLine_) / 20.0);
        setPos(total_ * v);
    } else if (timeLine_ <= 30) {
        setScale(1 - 0.02 * (30 - timeLine_));
    } else {
        return true;
    }
    return false;
}

void AnimCanvas::stopAnimate()
{
    killTimer(timer_);
    timer_ = 0;
    setRect(QRectF());
    PageCanvas * canvas = static_cast<PageCanvas*>(parentItem()->childItems().first());
    canvas->setBrush(QBrush());
    canvas->setRect(QRectF());
    canvas->setTransformOriginPoint(0, 0);
    canvas->setFlag(ItemHasNoContents, true);
    canvas->setFlag(ItemClipsChildrenToShape, false);
    scene()->setBackgroundBrush(brush());
    snapshot_ = QPixmap();
    if (canvasControl_) {
        canvasControl_->resource()->transform().disconnect(this);
        canvasControl_->metaObject()->invokeMethod(
                    canvasControl_, "setPosBarVisible", Q_ARG(bool,true));
        canvasControl_ = nullptr;
    }
    emit animateFinished();
}

QRectF AnimCanvas::animateRect(int dir, QPointF& off) const
{
    QRectF r = scene()->sceneRect();
    if (canvasControl_) {
        r = parentItem()->mapFromScene(scene()->sceneRect()).boundingRect();
    }
    if (dir & 1)
        off.setX(r.width());
    if (dir & 2)
        off.setX(-r.width());
    if (dir & 4)
        off.setY(r.height());
    if (dir & 8)
        off.setY(-r.height());
    return r.translated(off);
}

void AnimCanvas::setPos(const QPointF &pos)
{
    CanvasItem::setPos(pos);
    for (QGraphicsItem * sibling : parentItem()->childItems()) {
        if (sibling == this)
            break;
        sibling->setPos(pos);
    }
}

void AnimCanvas::setScale(qreal scale)
{
    CanvasItem::setScale(scale);
    for (QGraphicsItem * sibling : parentItem()->childItems()) {
        if (sibling == this)
            break;
        sibling->setScale(scale);
    }
}

void AnimCanvas::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    //qDebug() << "paint" << rect();
    CanvasItem::paint(painter, option, widget);
    painter->drawPixmap(rect(), snapshot_, QRectF());
}

void AnimCanvas::timerEvent(QTimerEvent *event)
{
    (void) event;
    if (animate())
        stopAnimate();
}

