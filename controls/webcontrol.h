#ifndef WEBCONTROL_H
#define WEBCONTROL_H

#include "core/widgetcontrol.h"

class WebControl : public WidgetControl
{
    Q_OBJECT

    Q_PROPERTY(bool layoutScale READ layoutScale WRITE setLayoutScale)
    Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)
    Q_PROPERTY(bool fitToContent READ fitToContent WRITE setFitToContent)

public:
    Q_INVOKABLE WebControl(ResourceView *res);

public:
    bool layoutScale() const;

    void setLayoutScale(bool b);

    bool keepAspectRatio() const;

    void setKeepAspectRatio(bool b);

    bool fitToContent() const;

    void setFitToContent(bool b);

protected:
    virtual QWidget * createWidget(ResourceView * res) override;

    virtual void attached() override;

private slots:
    void loadFinished(bool ok);

    void contentsSizeChanged(const QSizeF &size);

    void reload();

    void full();

    void fitContent();

    void debug();

private:
    bool fitToContent_;
};

#endif // WEBCONTROL_H
