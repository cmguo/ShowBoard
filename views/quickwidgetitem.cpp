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
        updateMask();
    } else if (change == ItemVisibleHasChanged) {
        for (QWidget* w : widgets_)
            w->setVisible(value.boolValue);
        updateMask();
    }
}

void QuickWidgetItem::updateMask()
{
    QRect rect = quickwidget_->rect();
    QRegion mask(rect);
    bool active = false;
    if (window() && isVisible()) {
        QRect rect2 = mapRectToScene(boundingRect()).toRect();
        mask = QRegion(rect) - QRegion(rect2);
        rect = rect2;
        active = true;
    }
    qDebug() << "QuickWidgetItem" << mask;
    quickwidget_->setMask(mask);
    QSize sz = rect.size();
    qDebug() << "QuickWidgetItem size" << sz;
    QPoint pt = rect.topLeft();
    qDebug() << "QuickWidgetItem pos" << pt;
    for (QWidget* w : widgets_) {
        w->resize(sz);
        w->move(pt);
    }
    if (active != active_) {
        active_ = active;
        onActiveChanged(active_);
    }
}
