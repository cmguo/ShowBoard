#ifndef WIDGETCONTROL_H
#define WIDGETCONTROL_H

#include "ShowBoard_global.h"

#include "core/control.h"

#include <QList>

class SHOWBOARD_EXPORT WidgetControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(bool touchable READ touchable WRITE setTouchable)
    Q_PROPERTY(QList<Qt::Key> overrideShotcuts MEMBER overrideShotcuts_)

public:
    WidgetControl(ResourceView *res, Flags flags = None, Flags clearFlags = None);

    virtual ~WidgetControl() override;

public:
    bool touchable() const;

    void setTouchable(bool b);

    QList<Qt::Key> overrideShotcuts() const { return overrideShotcuts_; }

    void setOverrideShotcuts(QList<Qt::Key> const & keys);

    QWidget * widget();

protected:
    virtual ControlView * create(ControlView * parent) override;

    virtual QWidget * createWidget(ControlView * parent) = 0;

    virtual void attaching() override;

    virtual void resize(QSizeF const & size) override;

    virtual void detached() override;

protected:
    virtual void bindTouchEventToChild(QWidget * child);

protected:
    virtual bool eventFilter(QObject * watched, QEvent * event) override;

protected:
    QWidget * widget_;
    QWidget * touchChild_;
    QList<Qt::Key> overrideShotcuts_;
};

#endif // WIDGETCONTROL_H
