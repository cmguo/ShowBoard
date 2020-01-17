#include "widgetcontrol.h"
#include "core/resource.h"
#include "core/resourceview.h"

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
    QGraphicsProxyWidget * item = new QGraphicsProxyWidget();
    item->setAcceptTouchEvents(true);
    item->setAutoFillBackground(false);
    item->setWidget(widget_);
    //resize(widget_->size());
    return item;
}

void WidgetControl::attaching()
{
    itemObj_ = widget_;
}

void WidgetControl::detached()
{
    static_cast<QGraphicsProxyWidget*>(item_)->setWidget(nullptr);
}

void WidgetControl::resize(QSizeF const & size)
{
    QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
    item->resize(size);
}
