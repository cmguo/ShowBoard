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

void QuickWidgetItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (!newGeometry.isEmpty())
        updateMask();
}

void QuickWidgetItem::windowDeactivateEvent()
{
    qDebug() << "QuickWidgetItem windowDeactivateEvent";
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
    if (window() && isVisible()) {
        QRect rect2 = mapRectToScene(boundingRect()).toRect();
        mask = QRegion(rect) - QRegion(rect2);
        rect = rect2;
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
}
