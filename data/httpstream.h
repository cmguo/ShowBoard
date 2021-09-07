#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include <QNetworkReply>

#include <functional>

class HttpStream : public QIODevice
{
    Q_OBJECT
public:
    HttpStream(QObject * context, QNetworkReply * reply);

    virtual ~HttpStream() override;

    QNetworkReply * reply() { return reply_; }

    static bool connect(QIODevice * stream, std::function<void()> finished,
                        std::function<void(std::exception &&)> error);

    static qint64 totalBytes(QIODevice * stream);

public:
    qint64 size() const override;

signals:
    void finished();

    void error(QNetworkReply::NetworkError);

private:
    void onError(QNetworkReply::NetworkError);

    void onReadyRead();

    void onFinished();

    void reopen();

    void pause();

    void resume();

    void abort();

protected:
    virtual qint64 readData(char *data, qint64 maxlen) override;
    virtual qint64 writeData(const char *data, qint64 len) override;
    virtual void timerEvent(QTimerEvent * event) override;

private:
    QNetworkReply * reply_;
    QNetworkReply * paused_;
    bool aborted_;
    qint64 lastPos_;
    qint64 speed_;
    int elapsed_; // in seconds
};


#endif // HTTPSTREAM_H
