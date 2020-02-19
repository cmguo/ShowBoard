#include "axthread.h"

#include <objbase.h>

AxThread::AxThread(char const * name)
    : WorkThread(name)
{
}

void AxThread::atexit(AxThread::exit_t exit)
{
    atexit_.append(exit);
}

void AxThread::run()
{
    CoInitialize(nullptr);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    exec();
    for (exit_t e : atexit_)
        e();
    CoUninitialize();
}
