#ifndef AVMEDIAPLAYERBRIDGE_H
#define AVMEDIAPLAYERBRIDGE_H

#include "mediaplayerbridge.h"

class AVMediaPlayerBridge : public MediaPlayerBridge
{
public:
    Q_INVOKABLE AVMediaPlayerBridge(QObject *parent = nullptr);

    virtual QObject *createMediaPlayer(QString url) override;

    virtual QWidget *createSurfaceView() override;
};

#endif // AVMEDIAPLAYERBRIDGE_H
