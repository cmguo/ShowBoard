#ifndef WHITECANVASQUICK_H
#define WHITECANVASQUICK_H

#include "quickwidgetitem.h"

class WhiteCanvasWidget;

class SHOWBOARD_EXPORT WhiteCanvasQuick : public QuickWidgetItem
{
    Q_OBJECT
public:
    WhiteCanvasQuick(WhiteCanvasWidget* canvas, QQuickWidget* quickwidget);

    void setUrl(QUrl const & url);

protected:
    virtual void onActiveChanged(bool active) override;

private:
    WhiteCanvasWidget * canvas_;
    QUrl mainUrl_;
};

#endif // WHITECANVASQUICK_H
