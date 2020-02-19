#ifndef WORD_H
#define WORD_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QPixmap>

class QAxObject;

class SHOWBOARD_EXPORT Word : public QObject
{
    Q_OBJECT
public:
    Word(QObject * parent = nullptr);

    virtual ~Word() override;

public:
    void open(QString const & file);

    void thumb(int page);

    void jump(int page);

    void prev();

    void next();

    void close();

public:
    int page()
    {
        return page_;
    }

    void setPage(int n)
    {
        page_ = n;
    }

signals:
    void opened(int total);

    void failed(QString const & msg);

    void thumbed(QPixmap const & pixmap);

    void closed();

private slots:
    void onPropertyChanged(const QString &name);

    void onSignal(const QString &name, int argc, void *argv);

    void onException(int code, const QString &source, const QString &desc, const QString &help);

private:
    void reopen();

private:
    static QAxObject * application_;

    static void quit();

private:
    QAxObject * documents_;
    QString file_;
    QAxObject * document_;
    QAxObject * panes_;
    int total_;
    int page_;
};

#endif // WORD_H
