#include "pageswitchevent.h"

PageSwitchEvent::PageSwitchEvent(Type type)
    : QEvent(type)
    , originEvent_(nullptr)
{
    setAccepted(false);
}

void PageSwitchEvent::setOriginEvent(QEvent *originEvent)
{
    originEvent_ = originEvent;
}

PageSwitchStartEvent::PageSwitchStartEvent(const QPointF &delta)
    : PageSwitchEvent(PageSwitchStart)
    , delta_(delta)
{
}

PageSwitchMoveEvent::PageSwitchMoveEvent(const QPointF &delta)
    : PageSwitchEvent(PageSwitchMove)
    , delta_(delta)
{
}

PageSwitchEndEvent::PageSwitchEndEvent()
    : PageSwitchEvent(PageSwitchEnd)
{
}
