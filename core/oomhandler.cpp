#include "oomhandler.h"

#include <QApplication>
#include <QEvent>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

static constexpr QEvent::Type EVENT_POST_HANDLE = QEvent::User;

OomHandler oomHandler;

static void oom_handler()
{
    oomHandler.handle();
}

static QMutex mutex;

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
        QWaitCondition c;
        QChildEvent event(EVENT_POST_HANDLE, reinterpret_cast<QObject*>(&c));
        QMutexLocker l(&mutex);
        QApplication::postEvent(this, &event);
        c.wait(&mutex);
        if (event.isAccepted())
            return;
    } else if (handle2()) {
        return;
    }
    throw std::runtime_error("内存不足");
}

bool OomHandler::event(QEvent *event)
{
    if (event->type() == EVENT_POST_HANDLE) {
        event->setAccepted(handle2());
        QMutexLocker l(&mutex);
        reinterpret_cast<QWaitCondition*>(
                    (static_cast<QChildEvent*>(event)->child()))->wakeOne();
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
