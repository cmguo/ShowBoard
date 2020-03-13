#include "canvasitem.h"

#include <QPainter>

CanvasItem::CanvasItem(QGraphicsItem * parent)
    : QGraphicsItem(parent)
{
    setFlag(ItemHasNoContents);
}

void CanvasItem::setRect(const QRectF &rect)
{
    prepareGeometryChange();
    rect_ = rect;
}

void CanvasItem::setBrush(const QBrush &brush)
{
    brush_ = brush;
}

QRectF CanvasItem::rect() const
{
    return rect_;
}

QBrush CanvasItem::brush() const
{
    return brush_;
}

QRectF CanvasItem::boundingRect() const
{
    return rect_;
}

void CanvasItem::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(brush_);
    painter->drawRect(rect_);
}
