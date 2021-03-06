#include "animcanvas.h"
#include "pagecanvas.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QDebug>
#include <QTimer>

AnimCanvas::AnimCanvas(CanvasItem * parent)
    : CanvasItem(parent)
    , canvasControl_(nullptr)
    , timer_(0)
    , timeLine_(0)
    , direction_(LeftToRight)
    , afterPageSwitch_(false)
    , switchPage_(false)
    , curve_(QEasingCurve::InOutQuart)
{
#ifdef SHOWBOARD_QUICK
#else
    setFlag(ItemHasNoContents, false);
#endif
    initAnimate();
}

AnimCanvas::~AnimCanvas()
{
    termAnimate();
}

void AnimCanvas::setSnapshot(QPixmap ss)
{
    snapshot_ = ss;
}

void AnimCanvas::setDirection(AnimateDirection dir)
{
    direction_ = dir;
}

void AnimCanvas::setAfterPageSwitch(bool after)
{
    afterPageSwitch_ = after;
    switchPage_ = after;
}

bool AnimCanvas::inAnimate() const
{
    return timer_;
}

void AnimCanvas::startAnimate()
{
    if (timer_)
        return;
    updateCanvas();
    initTranform();
    if (afterPageSwitch_) {
        setAnimPos(-total_);
        timer_ = startTimer(20);
    }
}

void AnimCanvas::stopAnimate()
{
    bool finished = timer_ == 0;
    if (timer_) {
        killTimer(timer_);
        timer_ = 0;
    }
    total_ = {0, 0};
    setAnimRect(QRectF(), {0, 0});
    setAnimPos({0, 0});
    snapshot_ = QPixmap();
    if (canvasControl_) {
        canvasControl_->resource()->transform().disconnect(this);
        canvasControl_->metaObject()->invokeMethod(
                    canvasControl_, "setPosBarVisible", Q_ARG(bool,true));
        canvasControl_ = nullptr;
    }
    emit animateFinished(finished);
}

void AnimCanvas::updateCanvas()
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

bool AnimCanvas::move(QPointF const & offset)
{
    assert(!afterPageSwitch_);
    QPointF pos = itemPosition(this);
    QPointF p = parentItem()->mapToScene(pos);
    bool valid = false;
    if (direction_ & (LeftToRight | RightToLeft) && !qFuzzyIsNull(offset.x())) {
        p.setX(p.x() + offset.x());
        valid = true;
    }
    if (direction_ & (TopToBottom | BottomToTop) && !qFuzzyIsNull(offset.y())) {
        p.setY(p.y() + offset.y());
        valid = true;
    }
    if (!valid)
        return true;
    QPointF q = parentItem()->mapFromScene(p);
    if (qAbs(q.x()) > qAbs(total_.x()))
        q.setX(total_.x());
    if (qAbs(q.y()) > qAbs(total_.y()))
        q.setY(total_.y());
    qreal s = (qAbs(q.x()) + qAbs(q.y())) / (qAbs(total_.x()) + qAbs(total_.y()));
    if ((q.x() < 0 && total_.x() > 0) || (q.x() > 0 && total_.x() < 0))
        s = -s;
    s = scale() - s;
    if (s < 0.9) {
        s = 0.9;
        q -= total_ * (s - scale());
    } else if (s > 1.0) {
        s = 1.0;
        q -= total_ * (s - scale());
    } else {
        q = {0, 0};
    }
    if ((q.x() < 0 && total_.x() > 0) || (q.x() > 0 && total_.x() < 0))
        q.setX(0);
    if ((q.y() < 0 && total_.y() > 0) || (q.y() > 0 && total_.y() < 0))
        q.setY(0);
    if (qFuzzyIsNull(q.x() - pos.x()) && qFuzzyIsNull(q.y() - pos.y()) && qFuzzyIsNull(s - scale()))
        return false;
    setAnimScale(s);
    setAnimPos(q);
    return true;
}

bool AnimCanvas::release()
{
    assert(!afterPageSwitch_);
    QPointF p = itemPosition(this);
    qreal t = qAbs(p.x()) + qAbs(p.y());
    qreal T = qAbs(total_.x()) + qAbs(total_.y());
    switchPage_ = t * 6.0 > T;
    timeLine_ = qFuzzyIsNull(t)
            ? static_cast<int>((1.0 - scale()) / 0.02)
            : (qFuzzyIsNull(t - T)
               ? 30 - static_cast<int>((1.0 - scale()) / 0.02)
               : static_cast<int>(t * 20 / T) + 5);
    qDebug() << "AnimCanvas::release" << timeLine_;
    if (timeLine_ == 0 || timeLine_ == 30) {
        QTimer::singleShot(0, this, &AnimCanvas::stopAnimate);
    } else {
        timer_ = startTimer(20);
    }
    return switchPage_;
}

void AnimCanvas::initAnimate()
{
    PageCanvas * canvas = static_cast<PageCanvas*>(parentItem()->childItems().first());
#ifdef SHOWBOARD_QUICK
#else
    setBrush(scene()->backgroundBrush());
    scene()->setBackgroundBrush(QBrush());
    canvas->setBrush(brush());
    canvas->setFlag(ItemHasNoContents, false);
#endif
    for (ControlView * sibling : parentItem()->childItems()) {
        if (!canvas->hasSubCanvas(static_cast<CanvasItem*>(sibling)))
            break;
        sibling->setFlag(ItemClipsChildrenToShape, true);
    }
}

void AnimCanvas::termAnimate()
{
    PageCanvas * canvas = static_cast<PageCanvas*>(parentItem()->childItems().first());
#ifdef SHOWBOARD_QUICK
#else
    canvas->setBrush(QBrush());
    canvas->setFlag(ItemHasNoContents, true);
#endif
    for (ControlView * sibling : parentItem()->childItems()) {
        if (!canvas->hasSubCanvas(static_cast<CanvasItem*>(sibling)))
            break;
        sibling->setFlag(ItemClipsChildrenToShape, false);
    }
#ifdef SHOWBOARD_QUICK
#else
    scene()->setBackgroundBrush(brush());
#endif
}

void AnimCanvas::initTranform()
{
    QPointF off;
    QRectF r = animateRect(off);
    //qDebug() << "AnimCanvas::initTranform" << direction_ << r << off;
    setAnimRect(r, afterPageSwitch_ ? -off : off);
    total_ = off;
}

void AnimCanvas::updateTransform()
{
    if (total_.isNull())
        return;
    QRectF old = rect();
    initTranform();
    QRectF r = rect();
    QPointF pos = itemPosition(this);
    pos *= r.width() / old.width();
    total_ *= r.width() / old.width();
    setAnimPos(pos);
    //qDebug() << "AnimCanvas::updateTransform" << direction_ << r << total_;
    update();
    for (ControlView * sibling : parentItem()->childItems()) {
        if (sibling == this)
            break;
        sibling->update();
    }
}

bool AnimCanvas::animate()
{
    if (switchPage_)
        ++timeLine_;
    else
        --timeLine_;
    if (timeLine_ <= 5) {
        setAnimScale(1 - 0.02 * timeLine_);
        if (!switchPage_ && timeLine_ == 5)
            setAnimPos({0, 0});
    } else if (timeLine_ <= 25) {
        qreal v = curve_.valueForProgress((timeLine_ - 5) / 20.0);
        setAnimPos(total_ * (afterPageSwitch_ ? (v - 1) : v));
        if (!switchPage_ && timeLine_ == 25)
            setAnimScale(0.9);
    } else if (timeLine_ <= 30) {
        setAnimScale(1 - 0.02 * (30 - timeLine_));
    }
    return timeLine_ == 0 || timeLine_ == 30;
}

void AnimCanvas::setAnimRect(const QRectF &rect, QPointF const & off)
{
    CanvasItem::setRect(rect);
    setTransformOriginPoint(rect.center());
    QRectF r = rect.translated(off);
    //qDebug() << "AnimCanvas::setRect" << rect << r;
    for (ControlView * sibling : parentItem()->childItems()) {
        if (sibling == this)
            break;
        if (!PageCanvas::isPageCanvas(sibling))
            continue;
        static_cast<PageCanvas*>(sibling)->setRect(r);
        sibling->setTransformOriginPoint(r.center());
    }
}

void AnimCanvas::setAnimPos(const QPointF &pos)
{
    //qDebug() << "AnimCanvas::setAnimPos" << pos;
    setItemPosition(this, pos);
    for (ControlView * sibling : parentItem()->childItems()) {
        if (sibling == this)
            break;
        if (!PageCanvas::isPageCanvas(sibling))
            continue;
        setItemPosition(sibling, pos);
    }
}

void AnimCanvas::setAnimScale(qreal scale)
{
    CanvasItem::setScale(scale);
    for (ControlView * sibling : parentItem()->childItems()) {
        if (sibling == this)
            break;
        if (!PageCanvas::isPageCanvas(sibling))
            continue;
        sibling->setScale(scale);
    }
}

QRectF AnimCanvas::animateRect(QPointF& off) const
{
    QRectF r = itemSceneRect(this);
    if (canvasControl_) {
        r = parentItem()->mapRectFromScene(itemSceneRect(this));
    }
    if (direction_ & LeftToRight) // we are at right
        off.setX(r.width());
    if (direction_ & RightToLeft) // we are at left
        off.setX(-r.width());
    if (direction_ & TopToBottom)
        off.setY(r.height());
    if (direction_ & BottomToTop)
        off.setY(-r.height());
    // we must set canvas rect at center (0, 0)
    if (afterPageSwitch_) {
        // for this:    (r + off) - off ~ r + off
        // for canvas:  r - off ~ r
        // amin:        -off ~ (0,0)
        return r.translated(off);
    } else {
        // for this:    r - off ~ r
        // for canvas:  r ~ r + off
        // amin:        (0,0) ~ off
        return r.translated(-off);
    }
}

#ifdef SHOWBOARD_QUICK

#else

void AnimCanvas::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    //qDebug() << "paint" << rect();
    CanvasItem::paint(painter, option, widget);
    painter->drawPixmap(rect(), snapshot_, QRectF());
}

#endif

void AnimCanvas::timerEvent(QTimerEvent *event)
{
    (void) event;
    if (animate()) {
        killTimer(timer_);
        timer_ = 0;
        stopAnimate();
    }
}

