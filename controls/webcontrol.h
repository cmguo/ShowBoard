#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "core/widgetcontrol.h"

class WebControl : public WidgetControl
{
    Q_OBJECT

    Q_PROPERTY(bool fitToContent READ fitToContent WRITE setFitToContent)

public:
    Q_INVOKABLE WebControl(ResourceView *res);

public:
    bool fitToContent() const;

    void setFitToContent(bool b);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual void attached() override;

private slots:
    void loadFinished(bool ok);

    void contentsSizeChanged(const QSizeF &size);

    void reload();

    void hide();

    void full();

    void fitContent();

    void debug();

private:
    bool fitToContent_;
};

#endif // WEBCONTROL_H
