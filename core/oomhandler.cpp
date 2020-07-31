#include "oomhandler.h"

#include <QApplication>
#include <QEvent>
#include <QThread>

static constexpr QEvent::Type EVENT_CLEAR_SESSION = QEvent::User;

OomHandler oomHandler;

static void oom_handler()
{
    oomHandler.handle();
}

OomHandler::OomHandler(QObject *parent) : QObject(parent)
{
    std::set_new_handler(&oom_handler);
}

void OomHandler::addHandler(int level, std::function<bool ()> handler)
{
    if (level < 0) level = 0;
    if (level > 5) level = 5;
    oom_handlers[level].append(handler);
}

void OomHandler::handle()
{
    if (QThread::currentThread() != thread()) {
        QEvent event(EVENT_CLEAR_SESSION);
        QApplication::sendEvent(this, &event);
        if (event.isAccepted())
            return;
    } else if (handle2()) {
        return;
    }
    throw std::bad_alloc();
}

bool OomHandler::event(QEvent *event)
{
    if (event->type() == EVENT_CLEAR_SESSION) {
        event->setAccepted(handle2());
    }
    return QObject::event(event);
}

bool OomHandler::handle2()
{
    for (int i = 0; i < 6; ++i) {
        for (auto & h : oom_handlers[i]) {
            if (h())
                return true;
        }
    }
    return false;
}
