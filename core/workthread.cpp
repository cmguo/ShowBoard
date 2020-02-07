#include "workthread.h"

#include <objbase.h>

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


void WorkThread::run()
{
    CoInitialize(nullptr);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    exec();
    CoUninitialize();
}
