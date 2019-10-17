#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "widgetcontrol.h"

class WebControl : public WidgetControl
{
    Q_OBJECT
public:
    Q_INVOKABLE WebControl(ResourceView *res);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;
};

#endif // WEBCONTROL_H
