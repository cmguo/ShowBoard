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

    void handle();

public:
    virtual bool event(QEvent * event) override;

private:
    bool handle2();

private:
    QList<std::function<bool(void)>> oom_handlers[6];
};

extern OomHandler oomHandler;

#endif // OOMHANDLER_H
