#include "workthread.h"

WorkThread::WorkThread(char const * name)
{
    if (name)
        setObjectName(name);
    context_ = new QObject;
    context_->moveToThread(this);
    start();
}

WorkThread::~WorkThread()
{
    quit();
    wait();
}
