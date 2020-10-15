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
    Q_PROPERTY(QWidget* widget WRITE setWidget)
    Q_PROPERTY(QList<QWidget*> widgets WRITE setWidgets)
public:
    QuickWidgetItem(QQuickItem * parent = nullptr);

    QuickWidgetItem(QWidget* widget, QQuickWidget* quickwidget, QQuickItem * parent = nullptr);

    QuickWidgetItem(QList<QWidget*> widgets, QQuickWidget* quickwidget, QQuickItem * parent = nullptr);

    virtual ~QuickWidgetItem() override;

public:
    void setWidget(QWidget* widget);

    void setWidgets(QList<QWidget*> widgets);

protected:
    virtual void onGeometryChanged(const QRect &newGeometry) override;

    virtual void onActiveChanged(bool active) override;

    virtual void quickWidgetChanged(QQuickWidget *quickwidget) override;

    QList<QWidget*> const & widgets() { return widgets_; }

private:
    QList<QWidget*> widgets_;
};

#endif // QUICKWIDGETITEM_H
