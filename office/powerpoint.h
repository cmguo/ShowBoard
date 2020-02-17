#ifndef POWERPOINT_H
#define POWERPOINT_H

#include <QObject>
#include <QPixmap>

class QAxObject;

class PowerPoint : public QObject
{
    Q_OBJECT
public:
    PowerPoint(QObject * parent = nullptr);

    virtual ~PowerPoint() override;

public:
    void open(QString const & file);

    void thumb(int page);

    void show(int page);

    void hide();

    void jump(int page);

    void prev();

    void next();

    void close();

    void mayStopped();

public:
    int slideNumber()
    {
        return slideNumber_;
    }

    void setSlideNumber(int n)
    {
        slideNumber_ = n;
    }

public:
    void attachButton(intptr_t hwnd, QPoint const & pos);

    void moveButton(intptr_t hwnd, QPoint const & diff);

signals:
    void opened(int total);

    void reopened();

    void failed(QString const & msg);

    void showed();

    void thumbed(QPixmap pixmap);

    void closed();

private slots:
    void onPropertyChanged(const QString &name);

    void onSignal(const QString &name, int argc, void *argv);

    void onException(int code, const QString &source, const QString &desc, const QString &help);

private:
    void reopen();

private:
    virtual void timerEvent(QTimerEvent * event) override;

private:
    static QAxObject * application_;

private:
    QAxObject * presentations_;
    QString file_;
    QAxObject * presentation_;
    int total_;
    int slideNumber_;
    int thumbNumber_;
    QAxObject * view_;
    intptr_t hwnd_;
    int timerId_;
};

#endif // POWERPOINT_H
