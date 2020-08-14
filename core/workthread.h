#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QEvent>
#include <QThread>
#include <QWaitCondition>

class QWaitCondition;

class WorkEventBase : public QEvent
{
public:
    WorkEventBase(QWaitCondition * c = nullptr);
    virtual ~WorkEventBase();
private:
    friend class WorkThread;
    QWaitCondition * c_;
};

template <typename T>
class WorkEvent : public WorkEventBase
{
public:
    WorkEvent(T const & f) : t_(f) {}
    virtual ~WorkEvent() { t_(); }
private:
    T t_;
};

class WorkThread : public QThread
{
public:
    WorkThread(char const * name = nullptr);

    virtual ~WorkThread() override;

public:
    template <typename Func>
    inline void sendWork(Func const & func) {
        sendWork(context_, func);
    }

    template <typename Func>
    inline void postWork(Func const & func) {
        postWork(context_, func);
    }

public:
    // send and wait
    template <typename Func>
    inline static void sendWork(QObject* context, Func const & func)
    {
        sendWork2(context, new WorkEvent<Func>(func));
    }

    template <typename Func>
    inline static void postWork(QObject* context, Func const & func)
    {
        postWork2(context, new WorkEvent<Func>(func));
    }

    static void quitAll();

private:
    static void sendWork2(QObject* context, WorkEventBase * e);

    static void postWork2(QObject* context, WorkEventBase * e);

private:
    QObject* context_;
};

#endif // WORKTHREAD_H
