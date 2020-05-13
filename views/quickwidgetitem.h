#ifndef QUICKWIDGETITEM_H
#define QUICKWIDGETITEM_H

#include "ShowBoard_global.h"

#include "quickproxyitem.h"

#include <QList>
#include <QQuickItem>

class QWidget;
class QQuickWidget;

class SHOWBOARD_EXPORT QuickWidgetItem : public QuickProxyItem
{
    Q_OBJECT
public:
    QuickWidgetItem(QWidget* widget, QQuickWidget* quickwidget);

    QuickWidgetItem(QList<QWidget*> widgets, QQuickWidget* quickwidget);

    virtual ~QuickWidgetItem() override;

protected:
    virtual void onGeometryChanged(const QRect &newGeometry) override;

    virtual void onActiveChanged(bool active) override;

    QList<QWidget*> const & widgets() { return widgets_; }

private:
    QList<QWidget*> widgets_;
};

#endif // QUICKWIDGETITEM_H
