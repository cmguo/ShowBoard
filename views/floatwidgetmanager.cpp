#include "floatwidgetmanager.h"

#include <QWidget>
#include <QVariant>

FloatWidgetManager *FloatWidgetManager::from(QWidget *main)
{
    QObject * m = main->property("FloatWidgetManager").value<QObject*>();
    if (m == nullptr) {
        m = new FloatWidgetManager(main);
        main->setProperty("FloatWidgetManager", QVariant::fromValue(m));
    }
    return qobject_cast<FloatWidgetManager*>(m);
}

FloatWidgetManager::FloatWidgetManager(QWidget *main)
    : main_(main)
{
    for (QObject * c : main->children())
        if (c->isWidgetType())
            widgetOn_ = qobject_cast<QWidget*>(c);
    saveStates_.append(0);
}

void FloatWidgetManager::addWidget(QWidget *widget, Flags flags)
{
    QWidget * under = widgetUnder();
    widget->setParent(main_);
    widget->stackUnder(under);
    if (flags) {
        relayout(widget, flags);
    }
    widget->show();
    widgets_.append(widget);
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
