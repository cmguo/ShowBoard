#include "qmlcontrol.h"
#include "core/resourceview.h"

#include <QQuickWidget>

QmlControl::QmlControl(ResourceView *res)
    : WidgetControl(res)
{
}

QWidget *QmlControl::createWidget(ResourceView *res)
{
    QQuickWidget * widget = new QQuickWidget;
    widget->setSource(res->url());
    return widget;
}
