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

void WidgetControl::layout(QRectF const &rect)
{
    QGraphicsProxyWidget * item = static_cast<QGraphicsProxyWidget*>(item_);
    item->resize(rect.size());
    item->setPos(rect.topLeft());
}

void WidgetControl::detached()
{
    static_cast<QGraphicsProxyWidget*>(item_)->setWidget(nullptr);
}

void WidgetControl::resize(QSizeF const & size)
{
    QRectF rect(QPointF(0, 0), size);
    rect.moveCenter(QPointF(0, 0));
    layout(rect);
}
