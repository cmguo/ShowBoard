#ifndef QMLCONTROL_H
#define QMLCONTROL_H

#include "core/widgetcontrol.h"

class QmlControl : WidgetControl
{
    Q_OBJECT
public:
    Q_INVOKABLE QmlControl(ResourceView *res);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

};

#endif // QMLCONTROL_H
