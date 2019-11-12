#ifndef WIDGETCONTROL_H
#define WIDGETCONTROL_H

#include "ShowBoard_global.h"

#include "control.h"

class SHOWBOARD_EXPORT WidgetControl : public Control
{
public:
    WidgetControl(ResourceView *res, Flags flags = None, Flags clearFlags = None);

    virtual ~WidgetControl() override;

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual QWidget * createWidget(ResourceView * res) = 0;

    virtual void resize(QSizeF const & size) override;

    virtual void detached() override;

protected:
    QWidget * widget_;
};

#endif // WIDGETCONTROL_H