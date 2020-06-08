#include "whitecanvasquick.h"
#include "whitecanvaswidget.h"
#include "whitecanvas.h"
#include "whitecanvasquick.h"
#include "core/resourcepackage.h"
#include "core/control.h"

WhiteCanvasQuick::WhiteCanvasQuick(WhiteCanvasWidget *canvas, QQuickWidget *quickwidget, QQuickItem * parent)
    : QuickWidgetItem(canvas, quickwidget, parent)
    , canvas_(canvas)
{
}

WhiteCanvasQuick::WhiteCanvasQuick(QQuickItem *parent)
    : WhiteCanvasQuick(WhiteCanvasWidget::mainInstance(), nullptr, parent)
{
}

WhiteCanvasQuick::~WhiteCanvasQuick()
{
    canvas_->package()->removeVirtualPage(mainUrl_);
}

void WhiteCanvasQuick::setUrl(const QString &url)
{
    setUrl(QUrl(url), urlSettings_);
}

void WhiteCanvasQuick::setUrl(const QUrl &url, QVariantMap settings)
{
    if (mainUrl_ == url)
        return;
    if (isActive())
        onActiveChanged(false);
    mainUrl_ = url;
    urlSettings_ = settings;
    if (isActive())
        onActiveChanged(true);
}

void WhiteCanvasQuick::setSetting(QVariantMap settings)
{
    urlSettings_ = settings;
}

void WhiteCanvasQuick::onGeometryChanged(const QRect &newGeometry)
{
    if (mainControl_) {
        QRectF rect = canvas_->mapToScene(newGeometry).boundingRect();
        rect = mainControl_->item()->parentItem()->mapFromScene(rect).boundingRect();
        if (mainGeometryProperty_.isEmpty())
            mainControl_->setGeometry(rect);
        else
            mainControl_->setProperty(mainGeometryProperty_, rect);
        return;
    }
    QuickWidgetItem::onGeometryChanged(newGeometry);
}

void WhiteCanvasQuick::onActiveChanged(bool active)
{
    if (mainUrl_.isEmpty() || widgets().empty())
        return;
    qDebug() << "WhiteCanvasQuick" << active << mainUrl_;
    if (active) {
        canvas_->package()->newVirtualPageOrBringTop(mainUrl_, urlSettings_);
        QVariant prop = urlSettings_.value(SETTINGS_SET_GEOMETRY_ON_MAIN_RESOURCE);
        if (mainControl_ == nullptr && prop.isValid()) {
            mainControl_ = canvas_->canvas()->findControl(mainUrl_);
            mainGeometryProperty_ = prop.toByteArray();
            onGeometryChanged(mapRectToScene(boundingRect()).toRect());
            emit changed();
        }
    } else {
        if (canvas_->package())
            canvas_->package()->showVirtualPage(mainUrl_, false);
        mainControl_ = nullptr;
        emit changed();
    }
}
