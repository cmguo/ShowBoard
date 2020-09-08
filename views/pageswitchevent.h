#ifndef PAGESWITCHEVENT_H
#define PAGESWITCHEVENT_H

#include <QEvent>
#include <QPointF>

class PageSwitchEvent : public QEvent
{
public:
    static constexpr Type PageSwitchStart = static_cast<Type>(User + 1);
    static constexpr Type PageSwitchMove = static_cast<Type>(User + 2);
    static constexpr Type PageSwitchEnd = static_cast<Type>(User + 3);

public:
    PageSwitchEvent(Type type);

    void setOriginEvent(QEvent * originEvent);

    QEvent * originEvent() const { return originEvent_; }

private:
    QEvent * originEvent_;
};

// Accept PageSwitchStartEvent if you will handle page switch

class PageSwitchStartEvent : public PageSwitchEvent
{
public:
    PageSwitchStartEvent(QPointF const & delta);

    QPointF const & delta() const { return delta_; }

private:
    QPointF const delta_;
};

// Accept PageSwitchMoveEvent if you are still showing page switch
// Reject it will cancel page switch

class PageSwitchMoveEvent : public PageSwitchEvent
{
public:
    PageSwitchMoveEvent(QPointF const & delta);

    QPointF const & delta() const { return delta_; }

private:
    QPointF const delta_;
};

// Accept PageSwitchMoveEvent if you have done (not cancel) page switch

class PageSwitchEndEvent : public PageSwitchEvent
{
public:
    PageSwitchEndEvent();
};

#endif // PAGESWITCHEVENT_H
