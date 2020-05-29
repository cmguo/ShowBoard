#ifndef WHITECANVASQUICK_H
#define WHITECANVASQUICK_H

#include "quickwidgetitem.h"

class WhiteCanvasWidget;

class Control;

class SHOWBOARD_EXPORT WhiteCanvasQuick : public QuickWidgetItem
{
    Q_OBJECT
public:
    WhiteCanvasQuick(WhiteCanvasWidget* canvas, QQuickWidget* quickwidget);

    virtual ~WhiteCanvasQuick() override;

public:
    static constexpr char const * SETTINGS_SET_GEOMETRY_ON_MAIN_RESOURCE
            = "setGeometryOnMainResource";

    void setUrl(QUrl const & url, QVariantMap settings);

protected:
    virtual void onGeometryChanged(const QRect &newGeometry) override;

    virtual void onActiveChanged(bool active) override;

private:
    WhiteCanvasWidget * canvas_;
    QUrl mainUrl_;
    QVariantMap urlSettings_;
    Control * mainControl_ = nullptr;
    QByteArray mainGeometryProperty_;
};

#endif // WHITECANVASQUICK_H
