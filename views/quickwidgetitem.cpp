#include "quickwidgetitem.h"

#include <QQuickWindow>
#include <QQuickWidget>
#include <QDebug>

QuickWidgetItem::QuickWidgetItem(QWidget *widget, QQuickWidget* quickwidget)
    : widgets_({widget})
    , quickwidget_(quickwidget)
{
}

QuickWidgetItem::QuickWidgetItem(QList<QWidget*> widgets, QQuickWidget* quickwidget)
    : widgets_(widgets)
    , quickwidget_(quickwidget)
{
}

QuickWidgetItem::~QuickWidgetItem()
{
    qDebug() << "QuickWidgetItem desctruct";
}

void QuickWidgetItem::onActiveChanged(bool active)
{
    for (QWidget* w : widgets_) {
        w->setVisible(active);
    }
}

void QuickWidgetItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    (void) oldGeometry;
    qDebug() << "QuickWidgetItem geometryChanged" << newGeometry;
    updateState();
}

void QuickWidgetItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value)
{
    qDebug() << "QuickWidgetItem itemChange" << change;
    (void) value;
    if (change == ItemSceneChange) {
        updateState();
    } else if (change == ItemVisibleHasChanged) {
        updateState();
    }
}

void QuickWidgetItem::updateState()
{
    QRect rect = quickwidget_->rect();
    if (rect.isEmpty())
        return;
    QRegion mask(rect);
    bool active = false;
    QRectF rect2 = boundingRect();
    if (window() && isVisible() && !rect2.isEmpty()) {
        qDebug() << "QuickWidgetItem" << rect2;
        QRect rect3 = mapRectToScene(rect2).toRect();
        mask = QRegion(rect) - QRegion(rect3);
        addOverlayItemRegion(mask);
        rect = rect3;
        active = true;
        quickwidget_->property("activeWidgetItem");
    } else if (!active_) {
        return;
    }
    qDebug() << "QuickWidgetItem setMask" << mask;
    quickwidget_->setMask(mask);
    qDebug() << "QuickWidgetItem setGeometry" << rect;
    for (QWidget* w : widgets_) {
        w->setGeometry(rect);
    }
    setActive(active);
}

void QuickWidgetItem::setActive(bool active)
{
    if (active != active_) {
        QVariant activeItem = quickwidget_->property("activeWidgetItem");
        if (!active_) {
            if (activeItem.isValid()) {
                qvariant_cast<QuickWidgetItem*>(activeItem)->setActive(false);
            }
            activeItem.setValue(this);
        } else if (qvariant_cast<QuickWidgetItem*>(activeItem) == this) {
            activeItem.clear();
        }
        quickwidget_->setProperty("activeWidgetItem", activeItem);
        active_ = active;
        onActiveChanged(active_);
    }
}

void QuickWidgetItem::addOverlayItemRegion(QRegion &region)
{
    addOverlayItemRegion(region, parentItem());
}

void QuickWidgetItem::addOverlayItemRegion(QRegion &region, QQuickItem *item)
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
            qDebug() << "QuickWidgetItem addOverlayItemRegion" << sibling << rect;
            region |= rect;
        }
    }
    addOverlayItemRegion(region, parent);
}
