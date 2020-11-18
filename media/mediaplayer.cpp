#include "mediaplayer.h"

MediaPlayer::MediaPlayer(QObject * parent):
    QObject(parent)
  ,volume_(1.0)
  ,mute_(false)
{

}

MediaPlayer::~MediaPlayer()
{
}

void MediaPlayer::showNextFrame()
{

}

QWidget *MediaPlayer::createRenderer()
{
    return nullptr;
}

void MediaPlayer::pause()
{

}

void MediaPlayer::removeRenderer(QWidget *)
{

}

