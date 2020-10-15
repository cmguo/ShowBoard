#include "canvasitem.h"

#include <QPainter>

CanvasItem::CanvasItem(CanvasView * parent)
    : CanvasView(parent)
{
#ifdef SHOWBOARD_QUICK
#else
    setFlag(ItemHasNoContents);
#endif
}

void CanvasItem::setRect(const QRectF &rect)
{
#ifdef SHOWBOARD_QUICK
#else
    prepareGeometryChange();
#endif
    rect_ = rect;
}

QRectF CanvasItem::rect() const
{
    return rect_;
}

#ifdef SHOWBOARD_QUICK
#else

void CanvasItem::setBrush(const QBrush &brush)
{
    brush_ = brush;
}

QBrush CanvasItem::brush() const
{
    return brush_;
}

#endif

QRectF CanvasItem::boundingRect() const
{
    return rect_;
}

#ifdef SHOWBOARD_QUICK
#else

void CanvasItem::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(brush_);
    painter->drawRect(rect_);
}

#endif
