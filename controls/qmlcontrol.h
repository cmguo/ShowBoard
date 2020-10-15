#ifndef QMLCONTROL_H
#define QMLCONTROL_H

#include "ShowBoard_global.h"

#include "controls/widgetcontrol.h"

class SHOWBOARD_EXPORT QmlControl : public WidgetControl
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
    virtual QWidget * createWidget(ControlView * parent) override;

    virtual void attached() override;

private slots:
    void hide();

    void update();
};

#include <QQuickImageProvider>
Q_DECLARE_METATYPE(QQuickImageProvider*)

#endif // QMLCONTROL_H
