#include "axthread.h"

#include <objbase.h>

AxThread::AxThread(char const * name)
    : WorkThread(name)
{
}

void AxThread::run()
{
    CoInitialize(nullptr);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    exec();
    CoUninitialize();
}
