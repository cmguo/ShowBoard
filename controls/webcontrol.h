#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "core/widgetcontrol.h"

class WebControl : public WidgetControl
{
    Q_OBJECT

    Q_PROPERTY(bool layoutScale READ layoutScale WRITE setLayoutScale)

public:
    Q_INVOKABLE WebControl(ResourceView *res);

public:
    bool layoutScale() const;

    void setLayoutScale(bool b);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual void attached() override;

private slots:
    void loadFinished(bool ok);

    void contentsSizeChanged(const QSizeF &size);

    void reload();

    void full();

    void debug();
};

#endif // WEBCONTROL_H
