#ifndef STROKECONTROL_H
#define STROKECONTROL_H

#include "widgetcontrol.h"

class GLStrokeWidget;

class StrokeControl : public WidgetControl
{
    Q_OBJECT
public:
    Q_INVOKABLE StrokeControl(ResourceView *res);

protected:
    virtual QWidget * createWidget(ControlView * parent) override;
};

#endif // STROKECONTROL_H
