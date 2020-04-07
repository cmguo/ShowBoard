#include "floatwidgetmanager.h"
#include "core/toolbutton.h"

#include <QWidget>
#include <QLayout>
#include <QVariant>
#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>

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
    connect(qobject_cast<QApplication*>(QApplication::instance()), &QApplication::focusChanged,
            this, &FloatWidgetManager::focusChanged);
}

void FloatWidgetManager::setTaskBar(QWidget *bar, int disableActions)
{
    taskBar_ = bar;
    disableActions_ = disableActions;
}

void FloatWidgetManager::addWidget(QWidget *widget, Flags flags)
{
    widget->raise();
    setWidgetFlags2(widget, flags);
    //widget->show();
    connect(widget, &QObject::destroyed, this, &FloatWidgetManager::removeDestroyWidget);
    widgets_.append(widget);
    if (!modifiedStates_.empty()) {
        modifiedStates_.back() |= 1 << widgets_.indexOf(widget);
    }
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
    bool fromDestroy = sender() == widget;
    widget->disconnect(this);
    setWidgetFlags2(widget, nullptr);
    //widget->hide();
    if (!fromDestroy)
        widget->setParent(nullptr);
    widgets_.removeOne(widget);
    widgetFlags_.remove(widget);
    int mask1 = (1 << n) - 1;
    int mask2 = static_cast<int>(uint(-1) << (n + 1));
    for (int & state : saveStates_) {
        state = (state & mask1) | ((state & mask2) >> 1);
    }
    for (int & state : modifiedStates_) {
        state = (state & mask1) | ((state & mask2) >> 1);
    }
}

void FloatWidgetManager::raiseWidget(QWidget *widget)
{
    int n = widgets_.indexOf(widget);
    if (n < 0)
        return;
    widget->raise();
    if (widget == widgets_.last())
        return;
    widgets_.removeAt(n);
    int mask1 = (1 << n) - 1;
    int mask2 = static_cast<int>(uint(-1) << (n + 1));
    int mask3 = 1 << n;
    n = widgets_.size() - n;
    for (int & state : saveStates_) {
        state = (state & mask1) | ((state & mask2) >> 1) | ((state & mask3) << n);
    }
    for (int & state : modifiedStates_) {
        state = (state & mask1) | ((state & mask2) >> 1) | ((state & mask3) << n);
    }
    widgets_.append(widget);
}

void FloatWidgetManager::lowerWidget(QWidget *widget)
{
    int n = widgets_.indexOf(widget);
    if (n < 0)
        return;
    if (widget == widgets_.first())
        return;
    widget->stackUnder(widgets_[1]);
    widgets_.removeAt(n);
    int mask1 = (1 << n) - 1;
    int mask2 = static_cast<int>(uint(-1) << (n + 1));
    int mask3 = 1 << n;
    for (int & state : saveStates_) {
        state = ((state & mask1) >> 1) | (state & mask2) | ((state & mask3) >> n);
    }
    for (int & state : modifiedStates_) {
        state = ((state & mask1) >> 1) | (state & mask2) | ((state & mask3) >> n);
    }
    widgets_.prepend(widget);
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
    qDebug() << "FloatWidgetManager::saveVisibility" << state;
    saveStates_.append(state);
    modifiedStates_.append(0);
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

void FloatWidgetManager::hideAll(QWidget* except)
{
    QStringList hides = except->property(HIDE_LIST).toStringList();
    for (QWidget * w : widgets_) {
        if (w == except)
            continue;
        if (!hides.isEmpty() && !hides.contains(w->objectName())
                 && !hides.contains(w->metaObject()->className()))
            continue;
        if (w->isVisible())
            qDebug() << "FloatWidgetManager::hideAll" << w;
        w->hide();
    }
}

void FloatWidgetManager::restoreVisibility()
{
    int state = saveStates_.takeLast();
    int modifiedState = modifiedStates_.takeLast();
    int mask = 1;
    for (QWidget * w : widgets_) {
        if ((modifiedState & mask) == 0 && w->isVisible() != ((state & mask) != 0)) {
            qDebug() << "FloatWidgetManager::restoreVisibility" << w << !w->isVisible();
            w->setVisible(state & mask);
        }
        mask <<= 1;
    }
}

void FloatWidgetManager::saveActionState()
{
    int state = 0;
    int mask = 1;
    for (QAction * a : taskBar_->actions()) {
        if (a->isEnabled())
            state |= mask;
        mask <<= 1;
    }
    saveActionStates_.append(state);
}

void FloatWidgetManager::disableActions()
{
    int state = disableActions_;
    int mask = 1;
    for (QAction * a : taskBar_->actions()) {
        if (a->isEnabled() && ((state & mask) != 0)) {
            qDebug() << "FloatWidgetManager::hideAll" << a;
            a->setEnabled(false);
        }
        mask <<= 1;
    }
}

void FloatWidgetManager::restoreActionState()
{
    int state = saveActionStates_.takeLast();
    int mask = 1;
    for (QAction * a : taskBar_->actions()) {
        if (a->isEnabled() != ((state & mask) != 0))
            qDebug() << "FloatWidgetManager::restoreActionState" << a << !a->isEnabled();
        a->setEnabled(state & mask);
        mask <<= 1;
    }
}

bool FloatWidgetManager::eventFilter(QObject *watched, QEvent *event)
{
    if (!main_->isActiveWindow())
        return false;
    QWidget * widget = qobject_cast<QWidget*>(watched);
    Flags flags = widgetFlags_.value(widget);
    if (event->type() == QEvent::Show) {
        if (!modifiedStates_.empty()) {
            modifiedStates_.back() |= 1 << widgets_.indexOf(widget);
        }
        if (flags.testFlag(RaiseOnFocus)) {
            widget->setFocus();
        } else {
            if (flags.testFlag(HideOnLostFocus))
                widget->setFocus();
            if (flags.testFlag(RaiseOnShow))
                raiseWidget(widget);
            else if (flags.testFlag(LowerOnShow))
                lowerWidget(widget);
        }
        if (flags.testFlag(HideOthersOnShow)) {
            saveVisibility();
            saveStates_.back() &= ~(1 << widgets_.indexOf(widget));
            modifiedStates_.back() |= 1 << widgets_.indexOf(widget);
            hideAll(widget);
        }
        if (flags.testFlag(DisableActionsOnShow)) {
            saveActionState();
            disableActions();
        }
    } else if (event->type() == QEvent::Hide) {
        if (!modifiedStates_.empty()) {
            modifiedStates_.back() |= 1 << widgets_.indexOf(widget);
        }
        if (flags.testFlag(DisableActionsOnShow)) {
            restoreActionState();
        }
        if (flags.testFlag(HideOthersOnShow)) {
            restoreVisibility();
        }
    }
    return false;
}

void FloatWidgetManager::setWidgetFlags2(QWidget *widget, Flags flags)
{
    relayout(widget, flags);
    if (flags.testFlag(RaiseOnShow) || flags.testFlag(LowerOnShow)
            || flags.testFlag(HideOnLostFocus)
            || flags.testFlag(HideOthersOnShow)) {
        widgetFlags_[widget] = flags;
        if (widget->isVisible()) {
            QEvent event(QEvent::Show);
            eventFilter(widget, &event);
        }
        widget->installEventFilter(this);
    } else {
        widget->removeEventFilter(this);
        if (widget->isVisible()) {
            QEvent event(QEvent::Hide);
            eventFilter(widget, &event);
        }
        widgetFlags_[widget] = flags;
    }
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
    //qDebug() << "FloatWidgetManager::focusChanged" << old << now;
    while (old && !widgets_.contains(old))
        old = old->parentWidget();
    while (now && !widgets_.contains(now))
        now = now->parentWidget();
    if (old == now) return;
    qDebug() << "FloatWidgetManager::focusChanged" << old << now;
    static QWidget * lastFocus = nullptr;
    static QElapsedTimer lastFocusTime;
    if (old && widgetFlags_.value(old).testFlag(HideOnLostFocus)) {
        if (lastFocus == old && !lastFocusTime.hasExpired(300)) {
            qDebug() << "FloatWidgetManager::focusChanged: ignore fast focus lost";
            old->setFocus();
            return;
        } else {
            old->hide();
        }
    }
    if (now) {
        lastFocus = now;
        lastFocusTime.start();
        if (widgetFlags_.value(now).testFlag(RaiseOnFocus)) {
            raiseWidget(now);
        }
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

void FloatWidgetManager::removeDestroyWidget()
{
    QWidget *widget = qobject_cast<QWidget*>(sender());
    if (widget) {
        qDebug() << "FloatWidgetManager::removeDestroyWidget" << widget;
        removeWidget(widget);
    }
}
