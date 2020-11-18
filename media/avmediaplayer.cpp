#include "avmediaplayer.h"

#include <QtAVWidgets/WidgetRenderer.h>

#define PRE_CAPTURE_ENABLE 0

using namespace QtAV;

AVMediaPlayer::AVMediaPlayer(QObject *parent):MediaPlayer(parent)
  ,preview_(nullptr)
{
    player_= new AVPlayer(parent);
    player_->setBufferMode(BufferMode::BufferPackets);
    connect(player_,&AVPlayer::stateChanged,this,&AVMediaPlayer::playingChanged);
    connect(player_,&AVPlayer::bufferProgressChanged,this,&AVMediaPlayer::bufferProgressChanged);
    connect(player_,&AVPlayer::sourceChanged,this,&AVMediaPlayer::sourceChanged);
    connect(player_,&AVPlayer::durationChanged,this,&AVMediaPlayer::durationChanged);
    connect(player_,&AVPlayer::speedChanged,this,&AVMediaPlayer::speedChanged);
    connect(player_,&AVPlayer::positionChanged,this,&AVMediaPlayer::positionChanged);

    connect(player_,&AVPlayer::stopped,this,&AVMediaPlayer::playingChanged);
    connect(player_,&AVPlayer::started,this,&AVMediaPlayer::playingChanged);
    connect(player_,&AVPlayer::paused,this,&AVMediaPlayer::playingChanged);
    connect(player_,&AVPlayer::mediaStatusChanged,this,&AVMediaPlayer::loadingChange);

    connect(player_->audio(), &AudioOutput::volumeChanged,this,&AVMediaPlayer::applyVolume, Qt::DirectConnection);
    connect(player_->audio(), &AudioOutput::muteChanged, this,&AVMediaPlayer::applyVolume, Qt::DirectConnection);
    connect(player_,&AVPlayer::started,this,&AVMediaPlayer::applyVolume);
}

AVMediaPlayer::~AVMediaPlayer()
{

    if(preview_)
        delete preview_;
}

void AVMediaPlayer::togglePlayPause()
{
    if(player_->isPlaying()){
        player_->pause(!player_->isPaused());
    }else {
        player_->play();
    }
}

void AVMediaPlayer::seek(qint64 value)
{
    player_->seek(value);
}

void AVMediaPlayer::onTimeSliderHover(int pos, int value)
{
#if PRE_CAPTURE_ENABLE
    QToolTip::showText(QPoint(pos, 0), QTime(0, 0, 0).addMSecs(value).toString(QString::fromLatin1("HH:mm:ss")));
    int w = 240;
    int h=135;
    if (!preview_){
        preview_ = new VideoPreviewWidget();
        preview_->setAutoDisplayFrame(true);
        preview_->setFile(player_->file());
        preview_->setWindowFlags(Qt::Tool |Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        preview_->resize(w, h);
        preview_->move(QPoint(pos, 0) + QPoint(w/2, 800));
    }
    preview_->setTimestamp(value);
    preview_->preview();
    preview_->show();
#endif
}

void AVMediaPlayer::onTimeSliderLeave()
{
#if PRE_CAPTURE_ENABLE
    if (!preview_)
    {
        return;
    }
    if (preview_->isVisible())
    {
        preview_->hide();
    }
#endif
}

qint64 AVMediaPlayer::position() const
{
    return player_->position();
}

void AVMediaPlayer::setPosition(qint64 pos)
{
    player_->setPosition(pos);
}


qreal AVMediaPlayer::bufferProgress()const
{
    return player_->bufferProgress();
}

qint64 AVMediaPlayer::duration() const
{
    return player_->duration();
}

bool AVMediaPlayer::isPlaying() const
{
    return player_->isPlaying()?!player_->isPaused():false;
}


qreal AVMediaPlayer::volume() const
{
    return volume_;
}

void AVMediaPlayer::setVolume(qreal value)
{
    if (volume_ < 0) {
        qWarning("volume must > 0");
        return;
    }
    if (qFuzzyCompare(volume_ + 1.0, value + 1.0))
        return;
    volume_ = value;
    Q_EMIT volumeChanged();
    applyVolume();
}

bool AVMediaPlayer::isMuted() const
{
    return mute_;
}

void AVMediaPlayer::setMuted(bool m)
{
    if (isMuted() == m)
        return;
    mute_ = m;
    Q_EMIT mutedChanged();
    applyVolume();
}

bool AVMediaPlayer::isLoading() const
{
    return player_->mediaStatus() == MediaStatus::BufferingMedia;
}

void AVMediaPlayer::showNextFrame()
{
    if(player_->isPlaying()){
        //todo 展示解决暂停 setoutput时黑屏问题
        player_->pause(!player_->isPaused());
        player_->pause(!player_->isPaused());
    }else {
        player_->play();
    }
}

void AVMediaPlayer::setUrl(QString url)
{
    player_->setFile(url);
}

QWidget *AVMediaPlayer::createRenderer()
{
    WidgetRenderer *vo = new WidgetRenderer();
    player_->addVideoRenderer(vo);
    return vo;
}

void AVMediaPlayer::removeRenderer(QWidget * widget)
{
    player_->removeVideoRenderer(static_cast<WidgetRenderer*>(widget));
}

void AVMediaPlayer::pause()
{
    player_->pause(true);
}

void AVMediaPlayer::applyVolume()
{
    AudioOutput *ao = player_->audio();
    if (!ao || !ao->isAvailable())
        return;
    if (!sender() || qobject_cast<AudioOutput*>(sender()) != ao) {
        ao->setVolume(volume());
        ao->setMute(isMuted());
        return;
    }
    setVolume(ao->volume());
    setMuted(ao->isMute());
}
