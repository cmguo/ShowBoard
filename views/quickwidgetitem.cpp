#include "quickwidgetitem.h"

#include <QQuickWindow>
#include <QQuickWidget>
#include <QDebug>

QuickWidgetItem::QuickWidgetItem(QWidget *widget, QQuickWidget* quickwidget)
    : QuickWidgetItem(QList<QWidget*>{widget}, quickwidget)
{
}

QuickWidgetItem::QuickWidgetItem(QList<QWidget*> widgets, QQuickWidget* quickwidget)
    : QuickProxyItem(quickwidget)
    , widgets_(widgets)
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

QuickWidgetItem::~QuickWidgetItem()
{
    qDebug() << "QuickWidgetItem" << objectName() << "desctruct";
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
