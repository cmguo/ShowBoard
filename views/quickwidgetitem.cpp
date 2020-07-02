#include "quickwidgetitem.h"

#include <QQuickWindow>
#include <QQuickWidget>
#include <QDebug>

QuickWidgetItem::QuickWidgetItem(QQuickItem *parent)
    : QuickWidgetItem({}, nullptr, parent)
{
}

QuickWidgetItem::QuickWidgetItem(QWidget *widget, QQuickWidget* quickwidget, QQuickItem * parent)
    : QuickWidgetItem(QList<QWidget*>{widget}, quickwidget, parent)
{
}

QuickWidgetItem::QuickWidgetItem(QList<QWidget*> widgets, QQuickWidget* quickwidget, QQuickItem * parent)
    : QuickProxyItem(quickwidget, parent)
{
    setWidgets(widgets);
}

QuickWidgetItem::~QuickWidgetItem()
{
}

void QuickWidgetItem::setWidget(QWidget *widget)
{
    setWidgets({widget});
}

void QuickWidgetItem::setWidgets(QList<QWidget *> widgets)
{
    assert(widgets_.isEmpty());
    widgets_ = widgets;
    if (quickWidget_)
        quickWidgetChanged(quickWidget_);
}

void QuickWidgetItem::onGeometryChanged(const QRect &newGeometry)
{
    for (QWidget* w : widgets_) {
        w->setGeometry(newGeometry);
    }
}

void QuickWidgetItem::onActiveChanged(bool active)
{
    for (QWidget* w : widgets_) {
        w->setVisible(active);
    }
}

void QuickWidgetItem::quickWidgetChanged(QQuickWidget *quickwidget)
{
    QWidget * commonParent = quickwidget->parentWidget();
    for (QWidget* w : widgets_) {
        QObject::connect(w, &QObject::destroyed, this, [this] () {
            widgets_.removeOne(qobject_cast<QWidget*>(sender()));
        });
        QWidget * p = w->parentWidget();
        while (commonParent) {
            while (p && p != commonParent) {
                p = p->parentWidget();
            }
            if (p) break;
            commonParent = commonParent->parentWidget();
            p = w->parentWidget();
        }
    }
    if (!commonParent)
        qDebug() << "QuickWidgetItem: not common parent widget";
    setCommonParent(commonParent);
}
