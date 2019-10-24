#include "widgetcontrol.h"

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QWidget>

WidgetControl::WidgetControl(ResourceView *res, Flags flags, Flags clearFlags)
    : Control(res, flags, clearFlags)
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
    widget_->setAttribute(Qt::WA_NoSystemBackground);
    QGraphicsProxyWidget * item = new QGraphicsProxyWidget();
    item->setFocusPolicy(Qt::NoFocus);
    item->setAutoFillBackground(false);
    item->setWidget(widget_);
    move(QPointF(widget_->width(), widget_->height()) / -2.0);
    return item;
}

void WidgetControl::relayout()
{
    if (flags_ & FullLayout) {
        QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
        QSizeF size = item->size();
        move(QPointF(size.width(), size.height()) / 2.0);
        size = item->parentItem()->boundingRect().size();
        item->resize(size);
        move(QPointF(size.width(), size.height()) / -2.0);
    }
}

void WidgetControl::detach()
{
    static_cast<QGraphicsProxyWidget*>(item_)->setWidget(nullptr);
    Control::detach();
}
