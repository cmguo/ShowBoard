#ifndef WIDGETCONTROL_H
#define WIDGETCONTROL_H

#include "ShowBoard_global.h"

#include "core/control.h"

class SHOWBOARD_EXPORT WidgetControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(bool touchable READ touchable WRITE setTouchable)
public:
    WidgetControl(ResourceView *res, Flags flags = None, Flags clearFlags = None);

    virtual ~WidgetControl() override;

public:
    bool touchable() const;

    void setTouchable(bool b);

    QWidget * widget();

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual QWidget * createWidget(ResourceView * res) = 0;

    virtual void attaching() override;

    virtual void resize(QSizeF const & size) override;

    virtual void detached() override;

protected:
    virtual bool eventFilter(QObject * watched, QEvent * event) override;

protected:
    QWidget * widget_;
};

#endif // WIDGETCONTROL_H
