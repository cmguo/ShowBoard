#ifndef QMLCONTROL_H
#define QMLCONTROL_H

#include "ShowBoard_global.h"

#include "controls/widgetcontrol.h"

#ifdef SHOWBOARD_QUICK
class SHOWBOARD_EXPORT QmlControl : public Control
#else
class SHOWBOARD_EXPORT QmlControl : public WidgetControl
#endif
{
    Q_OBJECT

    Q_PROPERTY(QVariantMap qmlProperties WRITE setQmlProperties)
    Q_PROPERTY(QVariantMap imageProviders WRITE setImageProviders)

public:
    Q_INVOKABLE QmlControl(ResourceView *res);

public:
    void setQmlProperties(QVariantMap const& properties);

    void setImageProviders(QVariantMap const& providers);

protected:
#ifdef SHOWBOARD_QUICK

    virtual ControlView * create(ControlView * parent) override;

#else

    virtual QWidget * createWidget(ControlView * parent) override;

#endif
    virtual void attached() override;

private slots:
    void hide();

    void update();
};

#include <QQuickImageProvider>
Q_DECLARE_METATYPE(QQuickImageProvider*)

#endif // QMLCONTROL_H
