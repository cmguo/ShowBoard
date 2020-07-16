  #include "workthread.h"

#include <QList>
#include <QMutex>

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
