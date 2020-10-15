#ifndef TEXTCONTROL_H
#define TEXTCONTROL_H

#include "widgetcontrol.h"

#include <QPoint>

class TextControl : public WidgetControl
{
    Q_OBJECT

public:
    Q_INVOKABLE TextControl(ResourceView *res);

protected:
    virtual QWidget * createWidget(ControlView * parent) override;

    virtual void attached() override;

    virtual void onText(QString text) override;

    virtual void copy(QMimeData & data) override;

private:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPoint lastPos_;
};

#endif // TEXTCONTROL_H
