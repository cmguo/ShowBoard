#ifndef MEDIAPLAYERBRIDGE_H
#define MEDIAPLAYERBRIDGE_H

#include <QObject>


class MediaPlayerBridge : public QObject
{
    Q_OBJECT
public:
    explicit MediaPlayerBridge(QObject *parent =nullptr);

    virtual QObject * createMediaPlayer(QString url) = 0;

    virtual QWidget * createSurfaceView() = 0;
};

#endif // MEDIAPLAYERBRIDGE_H
