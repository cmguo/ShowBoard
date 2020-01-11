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

public slots:
    void updateState();

protected:
    virtual void onActiveChanged(bool active);

    bool isActive() const { return active_; }

    QList<QWidget*> const & widgets() { return widgets_; }

private:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    virtual void itemChange(ItemChange change, const ItemChangeData & value) override;

private:
    void setActive(bool active);

    void addOverlayItemRegion(QRegion & region);

    static void addOverlayItemRegion(QRegion & region, QQuickItem* item);

private:
    QList<QWidget*> widgets_;
    QQuickWidget* quickwidget_;
    bool active_ = false;
};

#endif // QUICKWIDGETITEM_H
