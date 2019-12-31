#ifndef QUICKWIDGETITEM_H
#define QUICKWIDGETITEM_H

#include "ShowBoard_global.h"

#include <QList>
#include <QQuickItem>

class QWidget;
class QQuickWidget;

class SHOWBOARD_EXPORT QuickWidgetItem : public QQuickItem
{
    Q_OBJECT
public:
    QuickWidgetItem(QWidget* widget, QQuickWidget* quickwidget);

    QuickWidgetItem(QList<QWidget*> widgets, QQuickWidget* quickwidget);

    virtual ~QuickWidgetItem() override;

protected:
    virtual void onActiveChanged(bool active);

private:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    virtual void itemChange(ItemChange change, const ItemChangeData & value) override;

private:
    void updateMask();

private:
    QList<QWidget*> widgets_;
    QQuickWidget* quickwidget_;
    bool active_ = false;
};

#endif // QUICKWIDGETITEM_H
