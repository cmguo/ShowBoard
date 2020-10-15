#include "qmlcontrol.h"
#include "core/resourceview.h"
#include "quick/quickhelper.h"

#ifdef SHOWBOARD_QUICK
#include <QQuickWindow>
#else
#include <QQuickWidget>
#include <QGraphicsItem>
#endif
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

static char const * toolstr = ""
        "hide()|隐藏|Checkable|;"
        "update()|刷新||;"
        ;

QmlControl::QmlControl(ResourceView *res)
#ifdef SHOWBOARD_QUICK
    : Control(res, {Touchable})
#else
    : WidgetControl(res, {Touchable})
#endif
{
    setToolsString(toolstr);
}

void QmlControl::setQmlProperties(const QVariantMap &properties)
{
    if (flags_ & RestorePersisted)
        return;
    QVector<QQmlContext::PropertyPair> qmlProperties;
    for (auto iter = properties.begin(); iter != properties.end(); ++iter)
        qmlProperties.append({iter.key(), iter.value()});
#ifdef SHOWBOARD_QUICK
    qmlContext(item_)->setContextProperties(qmlProperties);
#else
    QQuickWidget * widget = static_cast<QQuickWidget *>(widget_);
    widget->rootContext()->setContextProperties(qmlProperties);
#endif
}

void QmlControl::setImageProviders(const QVariantMap &providers)
{
    if (flags_ & RestorePersisted)
        return;
#ifdef SHOWBOARD_QUICK
    QQmlEngine * engine = qmlEngine(item_);
#else
    QQmlEngine * engine = static_cast<QQuickWidget *>(widget_)->engine();
#endif
    for (auto iter = providers.begin(); iter != providers.end(); ++iter)
        engine->addImageProvider(iter.key(), iter.value().value<QQuickImageProvider*>());
}

#ifdef SHOWBOARD_QUICK

ControlView *QmlControl::create(ControlView *parent)
{
    QQmlComponent qc(qmlEngine(parent->window()));
    qc.loadUrl(res_->url());
    return qobject_cast<QQuickItem*>(qc.create());
}

#else

QWidget *QmlControl::createWidget(ControlView *)
{
    QQuickWidget * widget = new QQuickWidget;
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setClearColor(Qt::transparent);
    return widget;
}

#endif

void QmlControl::attached()
{
#ifdef SHOWBOARD_QUICK
#else
    QQuickWidget * widget = qobject_cast<QQuickWidget*>(widget_);
    widget->setSource(res_->url());
#endif
    loadFinished(true);
}

void QmlControl::hide()
{
    item_->setVisible(!item_->isVisible());
}

void QmlControl::update()
{
#ifdef SHOWBOARD_QUICK
    item_->update();
#else
    QQuickWidget * widget = qobject_cast<QQuickWidget*>(widget_);
    widget->rootObject()->update();
#endif
}
