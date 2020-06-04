#ifndef WHITECANVASQUICK_H
#define WHITECANVASQUICK_H

#include "quickwidgetitem.h"

class WhiteCanvasWidget;

class Control;

class SHOWBOARD_EXPORT WhiteCanvasQuick : public QuickWidgetItem
{
    Q_OBJECT

    Q_PROPERTY(Control* mainControl READ mainControl NOTIFY changed)
    Q_PROPERTY(QString url WRITE setUrl)
public:
    WhiteCanvasQuick(QQuickItem * parent = nullptr);

    WhiteCanvasQuick(WhiteCanvasWidget* canvas, QQuickWidget* quickwidget, QQuickItem * parent = nullptr);

    virtual ~WhiteCanvasQuick() override;

public:
    static constexpr char const * SETTINGS_SET_GEOMETRY_ON_MAIN_RESOURCE
            = "setGeometryOnMainResource";

    void setUrl(QString const & url);

    void setUrl(QUrl const & url, QVariantMap settings);

    Control* mainControl() { return mainControl_; }

signals:
    void changed();

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
