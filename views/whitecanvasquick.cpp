#include "whitecanvasquick.h"
#include "whitecanvaswidget.h"
#include "core/resourcepackage.h"
#include "core/resourcepage.h"

WhiteCanvasQuick::WhiteCanvasQuick(WhiteCanvasWidget *canvas, QQuickWidget *quickwidget, const QUrl &url)
    : QuickWidgetItem(canvas, quickwidget)
    , canvas_(canvas)
    , mainUrl_(url)
{
}

void WhiteCanvasQuick::onActiveChanged(bool active)
{
    if (active) {
        canvas_->package()->currentPage()->addResourceOrBringTop(mainUrl_);
    } else {
        canvas_->package()->showVirtualPage(canvas_->package()->findVirtualPage(mainUrl_), false);
    }
}
