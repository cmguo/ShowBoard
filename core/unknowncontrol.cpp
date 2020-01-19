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
    item->setPen(Qt::NoPen);
    return item;
}

void UnknownControl::attached()
{
    loadFinished(false, "未知资源类型");
}
