#ifndef AXTHREAD_H
#define AXTHREAD_H

#include "core/workthread.h"

class AxThread : public WorkThread
{
public:
    AxThread(char const * name = nullptr);

public:
    typedef void(*exit_t)();
    void atexit(exit_t exit);

private:
    virtual void run() override;

private:
    QList<exit_t> atexit_;
};

#endif // AXTHREAD_H
