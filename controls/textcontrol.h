#ifndef TEXTCONTROL_H
#define TEXTCONTROL_H

#include "core/widgetcontrol.h"

class TextControl : public WidgetControl
{
    Q_OBJECT

public:
    Q_INVOKABLE TextControl(ResourceView *res);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual void attached() override;

    virtual void onText(QString text) override;
};

#endif // TEXTCONTROL_H
