#include "whitecanvasquick.h"
#include "whitecanvaswidget.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"

WhiteCanvasQuick::WhiteCanvasQuick(WhiteCanvasWidget *canvas, QQuickWidget *quickwidget)
    : QuickWidgetItem(canvas, quickwidget)
    , canvas_(canvas)
{
}

void WhiteCanvasQuick::setUrl(const QUrl &url)
{
    if (mainUrl_ == url)
        return;
    if (isActive())
        onActiveChanged(false);
    mainUrl_ = url;
    if (isActive())
        onActiveChanged(true);
}

void WhiteCanvasQuick::onActiveChanged(bool active)
{
    if (mainUrl_.isEmpty())
        return;
    if (active) {
        canvas_->package()->currentPage()->addResourceOrBringTop(mainUrl_);
    } else {
        if (canvas_->package())
            canvas_->package()->showVirtualPage(canvas_->package()->findVirtualPage(mainUrl_), false);
    }
}
