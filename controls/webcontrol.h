#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "core/widgetcontrol.h"

class WebControl : public WidgetControl
{
    Q_OBJECT
public:
    Q_INVOKABLE WebControl(ResourceView *res);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual QString toolsString() const override;

private slots:
    void loadFinished();

    void contentsSizeChanged(const QSizeF &size);

    void reload();
};

#endif // WEBCONTROL_H
