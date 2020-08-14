  #include "workthread.h"

#include <QApplication>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

static QMutex& mutex()
{
    static QMutex m;
    return m;
}

static QList<WorkThread*>& threads()
{
    static QList<WorkThread*> l;
    return l;
}

WorkThread::WorkThread(char const * name)
{
    if (name)
        setObjectName(name);
    context_ = new QObject;
    context_->moveToThread(this);
    start();
    QMutexLocker l(&mutex());
    threads().append(this);
}

WorkThread::~WorkThread()
{
    QMutexLocker l(&mutex());
    threads().removeOne(this);
    quit();
    wait();
}

void WorkThread::quitAll()
{
    QMutexLocker l(&mutex());
    for (WorkThread * t : threads()) {
        t->quit();
        t->wait();
    }
}

void WorkThread::sendWork2(QObject* context, WorkEventBase *e)
{
    QWaitCondition c;
    e->c_ = &c;
    QMutexLocker l(&mutex());
    QApplication::postEvent(context, e);
    c.wait(&mutex());
}

void WorkThread::postWork2(QObject *context, WorkEventBase *e)
{
    QApplication::postEvent(context, e);
}

WorkEventBase::WorkEventBase(QWaitCondition *c)
    : QEvent(User), c_(c)
{
}

WorkEventBase::~WorkEventBase()
{
    if (c_) {
        QMutexLocker l(&mutex());
        c_->wakeOne();
    }
}
