#include "floatwidgetmanager.h"
#include "core/toolbutton.h"

#include <QWidget>
#include <QLayout>
#include <QVariant>
#include <QApplication>
#include <QDebug>

Q_DECLARE_METATYPE(FloatWidgetManager::Flags);

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
    , taskBar_(nullptr)
{
    for (QObject * c : main->children())
        if (c->isWidgetType())
            widgetOn_ = qobject_cast<QWidget*>(c);
    saveStates_.append(0);
    connect(qobject_cast<QApplication*>(QApplication::instance()), &QApplication::focusChanged,
            this, &FloatWidgetManager::focusChanged);
}

void FloatWidgetManager::setTaskBar(QWidget *bar)
{
    taskBar_ = bar;
}

void FloatWidgetManager::addWidget(QWidget *widget, Flags flags)
{
    widget->raise();
    setWidgetFlags2(widget, flags);
    //widget->show();
    widgets_.append(widget);
}

void FloatWidgetManager::addWidget(QWidget *widget, ToolButton *attachButton, Flags flags)
{
    addWidget(widget, flags);
    widget->move(popupPos(widget, attachButton));
}

void FloatWidgetManager::removeWidget(QWidget *widget)
{
    int n = widgets_.indexOf(widget);
    if (n < 0)
        return;
    widget->removeEventFilter(this);
    //widget->hide();
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
    widget->raise();
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

void FloatWidgetManager::setWidgetFlags(QWidget *widget, Flags flags)
{
    if (widgets_.contains(widget))
        setWidgetFlags2(widget, flags);
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

bool FloatWidgetManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        qobject_cast<QWidget*>(watched)->setFocus();
    }
    return false;
}

void FloatWidgetManager::setWidgetFlags2(QWidget *widget, Flags flags)
{
    relayout(widget, flags);
    widget->setProperty("FloatWidgetFlags", QVariant::fromValue(flags));
    if (flags.testFlag(RaiseOnShow) || flags.testFlag(HideOnLostFocus))
        widget->installEventFilter(this);
    else
        widget->removeEventFilter(this);
}

void FloatWidgetManager::relayout(QWidget *widget, Flags flags)
{
    QRect rect = main_->geometry();
    rect.setHeight(rect.height() - taskBar_->height());
    if (flags & FullLayout) {
        widget->setGeometry(rect);
    } else if (flags & PositionAtCenter) {
        QRect r = widget->geometry();
        r.moveCenter(rect.center());
        widget->setGeometry(r);
    }
}

void FloatWidgetManager::focusChanged(QWidget * old, QWidget *now)
{
    qDebug() << "FloatWidgetManager::focusChanged" << old << now;
    while (old && !widgets_.contains(old))
        old = old->parentWidget();
    while (now && !widgets_.contains(now))
        now = now->parentWidget();
    qDebug() << "FloatWidgetManager::focusChanged" << old << now;
    if (old == now) return;
    if (now) {
        Flags flags = now->property("FloatWidgetFlags").value<Flags>();
        if (flags.testFlag(RaiseOnFocus))
            raiseWidget(now);
    }
    if (old) {
        Flags flags = old->property("FloatWidgetFlags").value<Flags>();
        if (flags.testFlag(HideOnLostFocus))
            old->hide();
    }
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
    rect2.moveCenter(center - QPoint(0, rect2.height() / 2 + 8));
    QRect bound = main_->rect().adjusted(10, 0, -10, 0);
    QPoint off;
    if (rect2.x() < bound.x())
        off.setX(bound.x() - rect2.x());
    if (rect2.right() > bound.right())
        off.setX(bound.right() - rect2.right());
    return rect2.topLeft() + off;
}
