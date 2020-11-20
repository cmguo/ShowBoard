#include "avmediaplayer.h"
#include "avmediaplayerbridge.h"
#include "mediaplayer.h"

#include <QtAVWidgets/WidgetRenderer.h>
#include <QtQml>

AVMediaPlayerBridge::AVMediaPlayerBridge(QObject *parent):MediaPlayerBridge(parent)
{
    qmlRegisterType<MediaPlayer>("MediaPlayer",1,0,"MediaPlayer");
}

QObject *AVMediaPlayerBridge::createMediaPlayer(QString url)
{
    AVMediaPlayer * player = new AVMediaPlayer(parent());
    player->setSource(url);
    return player;
}

QWidget *AVMediaPlayerBridge::createSurfaceView()
{
    return new WidgetRenderer();
}
