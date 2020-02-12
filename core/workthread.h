#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QThread>

class WorkThread : public QThread
{
public:
    WorkThread(char const * name = nullptr);

    virtual ~WorkThread() override;

public:
    template <typename Func>
    inline static void postWork(QObject* context, Func func) {
        QObject signalSource;
        QObject::connect(&signalSource, &QObject::destroyed,
                         context, func, Qt::QueuedConnection);
    }

    template <typename Func>
    inline void postWork(Func func) {
        postWork(context_, func);
    }

private:
    QObject* context_;
};

#endif // WORKTHREAD_H
