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
    item->setPos(QPointF(item->size().width(), item->size().height()) / -2.0);
    return item;
}

void WidgetControl::relayout()
{
    if (flags_ & FullLayout) {
        QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
        QSizeF size = item->parentItem()->boundingRect().size();
        resize(size);
    }
}

void WidgetControl::detached()
{
    static_cast<QGraphicsProxyWidget*>(item_)->setWidget(nullptr);
    Control::detached();
}

void WidgetControl::resize(QSizeF const & size)
{
    QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
    item->resize(size);
    item->setPos(QPointF(size.width(), size.height()) / -2.0);
}
