#include "unknowncontrol.h"

#include <QGraphicsRectItem>
#include <QPen>
#include <QtPromise>

UnknownControl::UnknownControl(ResourceView *res)
    : Control(res, {FullSelect}, {CanRotate, CanScale})
{
}

ControlView *UnknownControl::create(ControlView *parent)
{
    (void) parent;
#ifdef SHOWBOARD_QUICK
    QQuickItem * item = nullptr;
#else
    QGraphicsRectItem* item = new QGraphicsRectItem;
    item->setPen(Qt::NoPen);
#endif
    return item;
}

void UnknownControl::attached()
{
    QtPromise::resolve().delay(1000).then([this, l = life()]() {
        if (l.isNull()) return;
        loadFinished(false, "未知资源类型");
    });
}
