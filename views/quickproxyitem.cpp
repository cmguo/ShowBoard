#include "quickproxyitem.h"

#include <QQuickWidget>
#include <QQuickWindow>

QuickProxyItem::QuickProxyItem(QQuickWidget *quickwidget, QQuickItem * parent)
    : QQuickItem(parent)
    , quickwidget_(quickwidget)
    , commonParent_(quickwidget_)
{
}

QuickProxyItem::~QuickProxyItem()
{
    qDebug() << "QuickProxyItem" << objectName() << "desctruct";
}

void QuickProxyItem::onActiveChanged(bool)
{
}

void QuickProxyItem::onGeometryChanged(const QRect &)
{
}

void QuickProxyItem::setCommonParent(QWidget *widget)
{
    commonParent_ = widget;
}

void QuickProxyItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    (void) newGeometry;
    (void) oldGeometry;
    //qDebug() << "QuickProxyItem" << objectName() << "geometryChanged" << newGeometry;
    updateState();
}

/*
static constexpr char const * changeNames[] {
    "ItemChildAddedChange",      // value.item
    "ItemChildRemovedChange",    // value.item
    "ItemSceneChange",           // value.window
    "ItemVisibleHasChanged",     // value.boolValue
    "ItemParentHasChanged",      // value.item
    "ItemOpacityHasChanged",     // value.realValue
    "ItemActiveFocusHasChanged", // value.boolValue
    "ItemRotationHasChanged",    // value.realValue
    "ItemAntialiasingHasChanged", // value.boolValue
    "ItemDevicePixelRatioHasChanged", // value.realValue
    "ItemEnabledHasChanged"      // value.boolValue
};*/

void QuickProxyItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value)
{
    //qDebug() << "QuickProxyItem" << objectName() << "itemChange" << changeNames[change];
    (void) value;
    if (change == ItemSceneChange) {
        if (quickwidget_ == nullptr && window()) {
            quickwidget_ = qobject_cast<QQuickWidget*>(window());
            if (quickwidget_)
                quickWidgetChanged(quickwidget_);
        }
        updateState();
    } else if (change == ItemVisibleHasChanged) {
        updateState();
    }
}

void QuickProxyItem::updateState()
{
    QRect rect = quickwidget_->rect();
    if (rect.isEmpty())
        return;
    QRegion mask(rect);
    bool active = false;
    QRectF rect2 = boundingRect();
    if (window() && isVisible() && !rect2.isEmpty()) {
        //qDebug() << "QuickProxyItem" << objectName() << rect2;
        QRect rect3 = mapRectToScene(rect2).toRect();
        mask = QRegion(rect) - QRegion(rect3);
        addOverlayItemRegion(mask);
        rect = rect3;
        active = true;
        quickwidget_->property("activeWidgetItem");
    } else if (!active_) {
        return;
    }
    //qDebug() << "QuickProxyItem" << objectName() << "setMask" << mask;
    QWidget * overlay = quickwidget_;
    while (overlay != commonParent_) {
        overlay->setMask(mask);
        overlay = overlay->parentWidget();
    }
    //() << "QuickProxyItem" << objectName() << "setGeometry" << rect;
    setGeometry(rect);
    setActive(active);
}

void QuickProxyItem::setGeometry(const QRect &newGeometry)
{
    if (active_)
        onGeometryChanged(newGeometry);
}

void QuickProxyItem::setActive(bool active)
{
    if (active != active_) {
        qDebug() << "QuickProxyItem" << objectName() << "setActive" << active;
        QVariant activeItem = quickwidget_->property("activeWidgetItem");
        if (!active_) {
            if (activeItem.isValid()) {
                qvariant_cast<QuickProxyItem*>(activeItem)->setActive(false);
            }
            activeItem.setValue(this);
        } else if (qvariant_cast<QuickProxyItem*>(activeItem) == this) {
            activeItem.clear();
        }
        quickwidget_->setProperty("activeWidgetItem", activeItem);
        active_ = active;
        onActiveChanged(active_);
    }
}

void QuickProxyItem::addOverlayItemRegion(QRegion &region)
{
    addOverlayItemRegion(region, parentItem());
}

void QuickProxyItem::addOverlayItemRegion(QRegion &region, QQuickItem *item)
{
    QQuickItem * parent = item->parentItem();
    if (!parent)
        return;
    qreal zi = item->z();
    bool above = false;
    for (QQuickItem* sibling : parent->childItems()) {
        if (sibling == item) {
            above = true;
            continue;
        }
        if (!sibling->isVisible() || sibling->z() < zi)
            continue;
        if (sibling->z() > zi || above) {
            QRect rect(sibling->mapRectToScene(sibling->boundingRect()).toRect());
            //qDebug() << "QuickProxyItem addOverlayItemRegion" << sibling << rect;
            region |= rect;
        }
    }
    addOverlayItemRegion(region, parent);
}
