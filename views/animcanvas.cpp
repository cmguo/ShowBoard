#include "animcanvas.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QDebug>

AnimCanvas::AnimCanvas(QGraphicsItem * parent)
    : CanvasItem(parent)
    , canvasControl_(nullptr)
    , animTimer_(0)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);
}

void AnimCanvas::startAnimate(int dir)
{
    if (animTimer_)
        return;
    updateAnimate();
    QPointF off;
    setRect(animateRect(dir, off));
    setPos(-off);
    animTimer_ = startTimer(20);
}

void AnimCanvas::updateAnimate()
{
    canvasControl_ = Control::fromItem(parentItem());
    if (canvasControl_) {
        connect(&canvasControl_->resource()->transform(),
                         &ResourceTransform::changed, this, [this] () {
            updateTransform();
        });
    }
    updateTransform();
}

void AnimCanvas::updateTransform()
{
    if (!animTimer_)
        return;
    int dir = 0;
    QPointF pos = this->pos();
    if (!qFuzzyIsNull(pos.x()))
        dir |= pos.x() < 0 ? 1 : 2;
    if (!qFuzzyIsNull(pos.y()))
        dir |= pos.y() < 0 ? 4 : 8;
    QRectF old = rect();
    QPointF off;
    setRect(animateRect(dir, off));
    qDebug() << "updateAnimate" << dir << rect();
    pos *= rect().width() / old.width();
    setPos(pos);
    update();
}

bool AnimCanvas::animate()
{
    QPointF p = pos();
    if (qFuzzyIsNull(p.x()) && qFuzzyIsNull(p.y())) {
        return true;
    }
    if (!qFuzzyIsNull(p.x())) {
        QPointF d{rect().width() / 20, 0};
        p = p.x() < 0 ? p + d : p - d;
    }
    if (!qFuzzyIsNull(p.y())) {
        QPointF d{0, rect().height() / 20};
        p = p.y() < 0 ? p + d : p - d;
    }
    qDebug() << "timerEvent" << p;
    setPos(p);
    return false;
}

void AnimCanvas::stopAnimate()
{
    killTimer(animTimer_);
    animTimer_ = 0;
    setRect(QRectF());
    snapshot_ = QPixmap();
    if (canvasControl_) {
        canvasControl_->resource()->transform().disconnect(this);
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

void AnimCanvas::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    qDebug() << "paint" << rect();
    painter->setPen(Qt::NoPen);
    painter->setBrush(scene()->backgroundBrush());
    painter->drawRect(rect());
    painter->drawPixmap(rect(), snapshot_, QRectF());
}

void AnimCanvas::timerEvent(QTimerEvent *event)
{
    (void) event;
    if (animate())
        stopAnimate();
}

