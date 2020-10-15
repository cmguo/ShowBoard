#include "controlview.h"

#ifdef SHOWBOARD_QUICK
#include <QQuickItem>
#include <QQuickWindow>
#else
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#endif

#include <QRectF>

QRectF itemSceneRect(ControlView const *item)
{
#ifdef SHOWBOARD_QUICK
   return QRectF(QPointF(), item->window()->size());
#else
   return item->scene()->sceneRect();
#endif
}

ControlView *itemFromWidget(QWidget *widget, ControlView *parent)
{
#ifdef SHOWBOARD_QUICK
    (void) widget;
    (void) parent;
    return nullptr;
#else
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(parent);
    proxy->setAcceptTouchEvents(true);
    proxy->setWidget(widget);
    return proxy;
#endif
}

QWidget *widgetFromItem(ControlView const *item)
{
#ifdef SHOWBOARD_QUICK
    (void) item;
    return nullptr;
#else
    return static_cast<QGraphicsProxyWidget const *>(item)->widget();
#endif
}

QPointF itemPosition(const ControlView *item)
{
#ifdef SHOWBOARD_QUICK
    return item->position();
#else
    return item->pos();
#endif
}

void setItemPosition(ControlView *item, const QPointF &pos)
{
#ifdef SHOWBOARD_QUICK
    item->setPosition(pos);
#else
    item->setPos(pos);
#endif
}
