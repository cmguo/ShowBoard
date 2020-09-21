#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QtPromise>

#include <QEvent>
#include <QThread>
#include <QWaitCondition>
#include <type_traits>

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

template <typename T, typename R>
class AsyncEvent : public WorkEventBase
{
public:
    AsyncEvent(T const & f, QtPromise::QPromiseResolve<R> r,
               QtPromise::QPromiseReject<R> j)
        : t_(f), r_(r), j_(j) {}
    virtual ~AsyncEvent()
    {
        try { r_(t_()); }
        catch (...) { j_(std::current_exception()); }
    }
private:
    T t_;
    QtPromise::QPromiseResolve<R> r_;
    QtPromise::QPromiseReject<R> j_;
};

template <typename T>
class AsyncEvent<T, void> : public WorkEventBase
{
public:
    AsyncEvent(T const & f, QtPromise::QPromiseResolve<void> r,
               QtPromise::QPromiseReject<void> j)
        : t_(f), r_(r), j_(j) {}
    virtual ~AsyncEvent()
    {
        try { t_(); r_(); }
        catch (...) { j_(std::current_exception()); }
    }
private:
    T t_;
    QtPromise::QPromiseResolve<void> r_;
    QtPromise::QPromiseReject<void> j_;
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

    template <typename Func>
    struct PromiseFunctor
    {
        using ResultType = typename std::result_of<Func(void)>::type;
        using PromiseType = QtPromise::QPromise<ResultType>;
    };

    template <typename Func>
    inline typename PromiseFunctor<Func>::PromiseType asyncWork(Func const & func) {
        return asyncWork(context_, func);
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

    template <typename Func>
    inline static typename PromiseFunctor<Func>::PromiseType asyncWork(QObject* context, Func const & func)
    {
        typedef typename std::result_of<Func(void)>::type Result;
        return QtPromise::QPromise<Result>([&] (QtPromise::QPromiseResolve<Result> resolve
                                           , QtPromise::QPromiseReject<Result> reject) {
            postWork2(context, new AsyncEvent<Func, Result>(func, resolve, reject));
        });
    }

    static void quitAll();

private:
    static void sendWork2(QObject* context, WorkEventBase * e);

    static void postWork2(QObject* context, WorkEventBase * e);

private:
    QObject* context_;
};

#endif // WORKTHREAD_H
