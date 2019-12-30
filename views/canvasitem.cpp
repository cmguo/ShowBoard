#include "canvasitem.h"

CanvasItem::CanvasItem(QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
}

void CanvasItem::setRect(const QRectF &rect)
{
    rect_ = rect;
}

QRectF CanvasItem::rect() const
{
    return rect_;
}

QRectF CanvasItem::boundingRect() const
{
    return rect_;
}

void CanvasItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Nothing to paint
}
