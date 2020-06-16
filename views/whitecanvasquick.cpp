#include "whitecanvasquick.h"
#include "whitecanvaswidget.h"
#include "whitecanvas.h"
#include "whitecanvasquick.h"
#include "core/resourcepackage.h"
#include "core/control.h"
#include "core/resourceview.h"

#include <QQuickWidget>

#include <core/resourcepage.h>

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
    QVariant prop = urlSettings_.value(SETTINGS_SET_GEOMETRY_ON_MAIN_RESOURCE);
    if (prop.isNull())
        mainGeometryProperty_ = "";
    else
        mainGeometryProperty_ = prop.toByteArray();
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
    if (mainGeometryProperty_.isNull())
        QuickWidgetItem::onGeometryChanged(newGeometry);
    else
        QuickWidgetItem::onGeometryChanged(quickWidget_->geometry());
}

void WhiteCanvasQuick::onActiveChanged(bool active)
{
    if (mainUrl_.isEmpty() || widgets().empty())
        return;
    qDebug() << "WhiteCanvasQuick" << active << mainUrl_;
    if (active) {
        canvas_->package()->newVirtualPageOrBringTop(mainUrl_, urlSettings_);
        if (mainControl_ == nullptr && !mainGeometryProperty_.isNull()) {
            mainControl_ = canvas_->canvas()->findControl(mainUrl_);
            connect(mainControl_, &QObject::destroyed, this, [this] () {
                if (mainControl_) {
                    connect(canvas_->package(), &ResourcePackage::currentPageChanged,
                            this, [this] (ResourcePage * page) {
                        if (page->isVirtualPage() && page->mainResource()->url() == mainUrl_) {
                            mainControl_ = canvas_->canvas()->findControl(mainUrl_);
                            canvas_->package()->disconnect(this);
                        }
                    });
                }
            });
            onGeometryChanged(mapRectToScene(boundingRect()).toRect());
            emit changed();
        }
    } else {
        mainControl_ = nullptr;
        if (canvas_->package())
            canvas_->package()->showVirtualPage(mainUrl_, false);
        emit changed();
    }
}
