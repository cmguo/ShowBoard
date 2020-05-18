#include "unknowncontrol.h"
#include "views/stateitem.h"

#include <QGraphicsRectItem>
#include <QPen>
#include <QtPromise>

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
    QtPromise::resolve().delay(1000).then([this, l = life()]() {
        if (l.isNull()) return;
        loadFinished(false, "未知资源类型");
    });
}
