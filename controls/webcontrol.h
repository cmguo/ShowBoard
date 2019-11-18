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

    virtual QString toolsString(QString const & parent) const override;

    virtual void attached() override;

private slots:
    void loadFinished(bool ok);

    void contentsSizeChanged(const QSizeF &size);

    void reload();
};

#endif // WEBCONTROL_H
