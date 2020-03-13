#include "floatwidgetmanager.h"
#include "core/toolbutton.h"

#include <QWidget>
#include <QLayout>
#include <QVariant>
#include <QApplication>
#include <QDebug>

FloatWidgetManager *FloatWidgetManager::from(QWidget *widget)
{
    QWidget * main = widget->window();
    QObject * m = main->property("FloatWidgetManager").value<QObject*>();
    if (m == nullptr) {
        m = new FloatWidgetManager(widget);
        main->setProperty("FloatWidgetManager", QVariant::fromValue(m));
    }
    return qobject_cast<FloatWidgetManager*>(m);
}

QPoint FloatWidgetManager::getPopupPosition(QWidget *widget, ToolButton *attachButton)
{
    QWidget* bar = attachButton->associatedWidgets().first();
    return from(bar)->popupPos(widget, attachButton);
}

FloatWidgetManager::FloatWidgetManager(QWidget *main)
    : QObject(main)
    , main_(main)
{
    for (QObject * c : main->children())
        if (c->isWidgetType())
            widgetOn_ = qobject_cast<QWidget*>(c);
    saveStates_.append(0);
    connect(qobject_cast<QApplication*>(QApplication::instance()), &QApplication::focusChanged,
            this, &FloatWidgetManager::focusChanged);
}

void FloatWidgetManager::addWidget(QWidget *widget, Flags flags)
{
    QWidget * under = widgetUnder();
    widget->setParent(main_);
    if (under)
        widget->stackUnder(under);
    else
        widget->raise();
    if (flags) {
        relayout(widget, flags);
    }
    widget->show();
    widgets_.append(widget);
}

void FloatWidgetManager::addWidget(QWidget *widget, ToolButton *attachButton)
{
    addWidget(widget);
    widget->move(popupPos(widget, attachButton));
}

void FloatWidgetManager::removeWidget(QWidget *widget)
{
    int n = widgets_.indexOf(widget);
    if (n < 0)
        return;
    widget->hide();
    widget->setParent(nullptr);
    widgets_.removeOne(widget);
    int mask1 = (1 << n) - 1;
    int mask2 = -1 << (n + 1);
    for (int & state : saveStates_) {
        state = (state & mask1) | ((state & mask2) >> 1);
    }
}

void FloatWidgetManager::raiseWidget(QWidget *widget)
{
    int n = widgets_.indexOf(widget);
    if (n < 0 || widget == widgets_.last())
        return;
    QWidget * under = widgetUnder();
    if (under)
        widget->stackUnder(under);
    else
        widget->raise();
    widgets_.removeAt(n);
    int mask1 = (1 << n) - 1;
    int mask2 = -1 << (n + 1);
    int mask3 = 1 << n;
    n = widgets_.size() - n + 1;
    for (int & state : saveStates_) {
        state = (state & mask1) | ((state & mask2) >> 1) | ((state & mask3) << n);
    }
    widgets_.append(widget);
}

void FloatWidgetManager::saveVisibility()
{
    int state = 0;
    int mask = 1;
    for (QWidget * w : widgets_) {
        if (w->isVisible())
            state |= mask;
        mask <<= 1;
    }
}

void FloatWidgetManager::showWidget(QWidget *widget)
{
    if (widgets_.contains(widget))
        widget->show();
}

void FloatWidgetManager::hideWidget(QWidget *widget)
{
    if (widgets_.contains(widget))
        widget->hide();
}

void FloatWidgetManager::hideAll()
{
    for (QWidget * w : widgets_)
        w->hide();
}

void FloatWidgetManager::restoreVisibility()
{
    int state = saveStates_.takeLast();
    int mask = 1;
    for (QWidget * w : widgets_) {
        w->setVisible(state & mask);
        mask <<= 1;
    }
}

QWidget *FloatWidgetManager::widgetUnder()
{
    QWidget * w = widgets_.isEmpty() ? widgetOn_ : widgets_.last();
    int n = main_->children().indexOf(w);
    w = nullptr;
    QList<QObject*> lst = main_->children();
    for (int i = n + 1; i < lst.size(); ++i) {
        if (lst.at(i)->isWidgetType()) {
            w = qobject_cast<QWidget*>(lst.at(i));
            break;
        }
    }
    return w;
}

void FloatWidgetManager::relayout(QWidget *widget, Flags flags)
{
    QRect rect = main_->geometry();
    rect.setHeight(rect.height() - taskBar_->height());
    if (flags & Full) {
        widget->setGeometry(rect);
    } else if (flags & Center) {
        QRect r = widget->geometry();
        r.moveCenter(rect.center());
        widget->setGeometry(r);
    }
}

void FloatWidgetManager::focusChanged(QWidget *, QWidget *now)
{
    while (now && !widgets_.contains(now))
        now = now->parentWidget();
    if (now)
        raiseWidget(now);
}

QPoint FloatWidgetManager::popupPos(QWidget *widget, ToolButton *attachButton)
{
    QWidget* bar = attachButton->associatedWidgets().first();
    QRectF rect = attachButton->itemRect();
    if (rect.isEmpty())
        rect.setSize(bar->size());
    rect.setHeight(0);
    QPoint center = bar->mapTo(main_, rect.center().toPoint());
    QRect rect2 = widget->rect();
    rect2.moveCenter(center - QPoint(0, rect2.height() / 2 + 20));
    QRect bound = main_->rect().adjusted(10, 0, -10, 0);
    QPoint off;
    if (rect2.x() < bound.x())
        off.setX(bound.x() - rect2.x());
    if (rect2.right() > bound.right())
        off.setX(bound.right() - rect2.right());
    return rect2.topLeft() + off;
}
