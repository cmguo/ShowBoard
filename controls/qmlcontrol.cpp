#include "qmlcontrol.h"
#include "core/resourceview.h"

#include <QQuickWidget>
#include <QQuickItem>
#include <QGraphicsItem>
#include <QQmlContext>
#include <QQmlEngine>

static char const * toolstr = ""
        "hide()|隐藏|Checkable|;"
        "update()|刷新||;"
        ;

QmlControl::QmlControl(ResourceView *res)
    : WidgetControl(res, {Touchable})
{
    setToolsString(toolstr);
}

void QmlControl::setQmlProperties(const QVariantMap &properties)
{
    if (flags_ & RestorePersisted)
        return;
    QQuickWidget * widget = static_cast<QQuickWidget *>(widget_);
    QVector<QQmlContext::PropertyPair> qmlProperties;
    for (auto iter = properties.begin(); iter != properties.end(); ++iter)
        qmlProperties.append({iter.key(), iter.value()});
    widget->rootContext()->setContextProperties(qmlProperties);
}

void QmlControl::setImageProviders(const QVariantMap &providers)
{
    if (flags_ & RestorePersisted)
        return;
    QQuickWidget * widget = static_cast<QQuickWidget *>(widget_);
    for (auto iter = providers.begin(); iter != providers.end(); ++iter)
        widget->engine()->addImageProvider(iter.key(), iter.value().value<QQuickImageProvider*>());
}

QWidget *QmlControl::createWidget(ControlView *)
{
    QQuickWidget * widget = new QQuickWidget;
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setClearColor(Qt::transparent);
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
