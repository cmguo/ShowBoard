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
    qDebug() << "QuickWidgetItem geometryChanged" << newGeometry;
    if (!newGeometry.isEmpty())
        updateMask();
}

void QuickWidgetItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & value)
{
    qDebug() << "QuickWidgetItem itemChange" << change;
    (void) value;
    if (change == ItemSceneChange) {
        if (window())
            updateMask();
    } else if (change == ItemVisibleHasChanged) {
        updateMask();
    }
}

void QuickWidgetItem::updateMask()
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
    }
    qDebug() << "QuickWidgetItem setMask" << mask;
    quickwidget_->setMask(mask);
    qDebug() << "QuickWidgetItem setGeometry" << rect;
    for (QWidget* w : widgets_) {
        w->setGeometry(rect);
    }
    if (active != active_) {
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
