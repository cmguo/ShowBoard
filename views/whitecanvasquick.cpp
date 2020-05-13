#include "whitecanvasquick.h"
#include "whitecanvaswidget.h"
#include "whitecanvas.h"
#include "core/resourcepackage.h"
#include "core/control.h"

WhiteCanvasQuick::WhiteCanvasQuick(WhiteCanvasWidget *canvas, QQuickWidget *quickwidget)
    : QuickWidgetItem(canvas, quickwidget)
    , canvas_(canvas)
{
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

void WhiteCanvasQuick::onGeometryChanged(const QRect &newGeometry)
{
    if (mainControl_) {
        QRectF rect = canvas_->mapToScene(newGeometry).boundingRect();
        rect = mainControl_->item()->parentItem()->mapFromScene(rect).boundingRect();
        mainControl_->setGeometry(rect);
        return;
    }
    QuickWidgetItem::onGeometryChanged(newGeometry);
}

void WhiteCanvasQuick::onActiveChanged(bool active)
{
    if (mainUrl_.isEmpty() || widgets().empty())
        return;
    if (active) {
        canvas_->package()->newVirtualPageOrBringTop(mainUrl_, urlSettings_);
        if (urlSettings_.value(SETTINGS_SET_GEOMETRY_ON_MAIN_RESOURCE).toBool() == true)
            mainControl_ = canvas_->canvas()->findControl(mainUrl_);
    } else {
        if (canvas_->package())
            canvas_->package()->showVirtualPage(mainUrl_, false);
    }
}
