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

QWidget *QmlControl::createWidget(ResourceView *)
{
    QQuickWidget * widget = new QQuickWidget;
    return widget;
}

void QmlControl::attached()
{
    QQuickWidget * widget = qobject_cast<QQuickWidget*>(widget_);
    widget->setSource(res_->url());
    loadFinished(true);
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
