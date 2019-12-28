#include "unknowncontrol.h"
#include "views/stateitem.h"

#include <QGraphicsRectItem>
#include <QPen>

UnknownControl::UnknownControl(ResourceView *res)
    : Control(res, {FullSelect}, {CanRotate, CanScale})
{
}

QGraphicsItem *UnknownControl::create(ResourceView *res)
{
    (void) res;
    QGraphicsRectItem* item = new QGraphicsRectItem;
    //item->setPen(Qt::NoPen);
    return item;
}

void UnknownControl::attached()
{
    loadFinished(false, "未知资源类型");
    QGraphicsRectItem * item = static_cast<QGraphicsRectItem*>(item_);
    QRectF rect(stateItem()->boundingRect());
    QPointF center(rect.center());
    rect.adjust(0, 0, qAbs(center.x()) * 2, qAbs(center.y()) * 2);
    rect.moveCenter(QPointF(0, 0));
    item->setRect(rect);
}
