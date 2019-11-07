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
    widget_->setAttribute(Qt::WA_NoSystemBackground);
    QGraphicsProxyWidget * item = new QGraphicsProxyWidget();
    item->setFocusPolicy(Qt::NoFocus);
    item->setAutoFillBackground(false);
    item->setWidget(widget_);
    //resize(widget_->size());
    QSizeF size = item->size() / -2.0;
    item->setTransform(QTransform::fromTranslate(size.width(), size.height()));
    return item;
}

void WidgetControl::detached()
{
    static_cast<QGraphicsProxyWidget*>(item_)->setWidget(nullptr);
}

void WidgetControl::resize(QSizeF const & size)
{
    QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
    item->resize(size);
    item->setTransform(QTransform::fromTranslate(
                           size.width() / -2.0, size.height() / -2.0));
}
