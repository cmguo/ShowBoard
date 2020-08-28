#ifndef OOMHANDLER_H
#define OOMHANDLER_H

#include <QObject>

#include <functional>

class OomHandler : public QObject
{
    Q_OBJECT
public:
    explicit OomHandler(QObject *parent = nullptr);

public:
    void addHandler(int level, std::function<bool(void)> handler);

    static bool isMemoryAvailable(quint64 size);

    static void ensureMemoryAvailable(quint64 size);

public:
    virtual bool event(QEvent * event) override;

private:
    friend void oom_handler();

    void handle();

    bool handle2();

private:
    QList<std::function<bool(void)>> oom_handlers[6];
};

extern OomHandler oomHandler;

#endif // OOMHANDLER_H
