#include "whitecanvasquick.h"
#include "whitecanvaswidget.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"

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

void WhiteCanvasQuick::onActiveChanged(bool active)
{
    if (mainUrl_.isEmpty() || widgets().empty())
        return;
    if (active) {
        canvas_->package()->currentPage()->addResourceOrBringTop(mainUrl_, urlSettings_);
    } else {
        if (canvas_->package())
            canvas_->package()->showVirtualPage(mainUrl_, false);
    }
}
