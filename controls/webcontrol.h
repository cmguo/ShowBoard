#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "core/widgetcontrol.h"

class WebControl : public WidgetControl
{
    Q_OBJECT

    Q_PROPERTY(QSizeF sizeHint READ sizeHint  WRITE setSizeHint)

public:
    Q_INVOKABLE WebControl(ResourceView *res);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual QString toolsString() const override;

    virtual void attached() override;

protected:
    QSizeF sizeHint();

    void setSizeHint(QSizeF const & size);

private slots:
    void loadFinished(bool ok);

    void contentsSizeChanged(const QSizeF &size);

    void reload();
};

#endif // WEBCONTROL_H
