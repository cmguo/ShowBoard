#include "qmlcontrol.h"
#include "core/resourceview.h"

#include <QQuickWidget>
#include <QQuickItem>
#include <QGraphicsItem>

static char const * toolstr = ""
        "hide()|隐藏|Checkable|;"
        "update()|刷新||;"
        ;

QmlControl::QmlControl(ResourceView *res)
    : WidgetControl(res)
{
    setToolsString(toolstr);
}

QWidget *QmlControl::createWidget(ResourceView *res)
{
    QQuickWidget * widget = new QQuickWidget;
    widget->setSource(res->url());
    QObject::connect(widget->rootObject(), &QQuickItem::widthChanged,
                     this, [this]() { sizeChanged(); }, Qt::QueuedConnection);
    QObject::connect(widget->rootObject(), &QQuickItem::heightChanged,
                     this, [this]() { sizeChanged(); }, Qt::QueuedConnection);
    return widget;
}

void QmlControl::hide()
{
    item_->setVisible(!item_->isVisible());
}

void QmlControl::update()
{
    QQuickWidget * widget = qobject_cast<QQuickWidget*>(widget_);
    widget->rootObject()->update();
}
