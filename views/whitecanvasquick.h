#ifndef WHITECANVASQUICK_H
#define WHITECANVASQUICK_H

#include "quickwidgetitem.h"

class WhiteCanvasWidget;

class SHOWBOARD_EXPORT WhiteCanvasQuick : public QuickWidgetItem
{
public:
    WhiteCanvasQuick(WhiteCanvasWidget* canvas, QQuickWidget* quickwidget, QUrl const & url);

protected:
    virtual void onActiveChanged(bool active) override;

private:
    WhiteCanvasWidget * canvas_;
    QUrl mainUrl_;
};

#endif // WHITECANVASQUICK_H
