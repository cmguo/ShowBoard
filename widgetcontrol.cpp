#include "widgetcontrol.h"

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QWidget>

WidgetControl::WidgetControl(ResourceView *res, Flags flags)
    : Control(res, flags)
{
}

WidgetControl::~WidgetControl()
{
    widget_->deleteLater();
    widget_ = nullptr;
}

QGraphicsItem * WidgetControl::create(ResourceView *res)
{
    res_ = res;
    widget_ = createWidget(res);
    QGraphicsProxyWidget * item = new QGraphicsProxyWidget();
    item->setWidget(widget_);
    move(QPointF(widget_->width(), widget_->height()) / -2.0);
    return item;
}

void WidgetControl::detach()
{
    static_cast<QGraphicsProxyWidget*>(item_)->setWidget(nullptr);
    Control::detach();
}
