#ifndef AXTHREAD_H
#define AXTHREAD_H

#include "core/workthread.h"

class AxThread : public WorkThread
{
public:
    AxThread(char const * name = nullptr);

private:
    virtual void run() override;
};

#endif // AXTHREAD_H
