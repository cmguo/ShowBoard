#include "avmediaplayer.h"

#include <QtAVWidgets/WidgetRenderer.h>
#include <QTimer>
#include <QUrl>

#define PRE_CAPTURE_ENABLE 0

using namespace QtAV;

AVMediaPlayer::AVMediaPlayer(QObject *parent):AVPlayer(parent)
  ,volume_(1.0)
  ,mute_(false)
  ,autoPlay_(false)
  ,preparedState_(false)
  ,loaded_(false)
  ,preview_(nullptr)
{
    setAsyncLoad(true);
    setAutoLoad(true);
    setBufferMode(BufferMode::BufferPackets);
    connect(this,&AVPlayer::stateChanged,this,&AVMediaPlayer::videoStateChanged);
    connect(this,&AVPlayer::stopped,this,&AVMediaPlayer::videoStateChanged);
    connect(this,&AVPlayer::started,this,&AVMediaPlayer::videoStateChanged);
    connect(this,&AVPlayer::paused,this,&AVMediaPlayer::videoStateChanged);
    connect(this,&AVPlayer::mediaStatusChanged,this,&AVMediaPlayer::videoStateChanged);
    connect(this,&AVPlayer::mediaStatusChanged,this,&AVMediaPlayer::positionChanged);
    connect(this,&AVPlayer::positionChanged,this,&AVMediaPlayer::positionChanged);
    connect(audio(), &AudioOutput::volumeChanged,this,&AVMediaPlayer::applyVolume, Qt::DirectConnection);
    connect(audio(), &AudioOutput::muteChanged, this,&AVMediaPlayer::applyVolume, Qt::DirectConnection);
    connect(this,&AVPlayer::started,this,&AVMediaPlayer::applyVolume);
    connect(this,&AVPlayer::started,this,[this](){ preparedState_ = false;});
    connect(this,&AVPlayer::loaded,this,[this](){
        if(loaded_)
            return;
        loaded_ = true;
        preparedState_ = true;
        play();
        QTimer::singleShot(position()>50 ? 200 : 0,this,[=](){ //恢复进度不需要前进太多
            pause(true);
        });

        emit videoSizeChanged();
        emit videoStateChanged();
    });
}

AVMediaPlayer::~AVMediaPlayer()
{
    stop();
}
#if PRE_CAPTURE_ENABLE
void AVMediaPlayer::onTimeSliderHover(int pos, int value)
{

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

}


void AVMediaPlayer::onTimeSliderLeave()
{
    if (!preview_)
    {
        return;
    }
    if (preview_->isVisible())
    {
        preview_->hide();
    }
}
#endif


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

QString AVMediaPlayer::source() const
{
    return file();
}

void AVMediaPlayer::setSource(const QString source)
{
    QUrl url(source);
    if (url.isLocalFile() || url.scheme().isEmpty()
            || url.scheme().startsWith("qrc"))
        setFile(QUrl::fromPercentEncoding(url.toEncoded()));
    else
        setFile(url.toEncoded());
    load();
}

QWidget *AVMediaPlayer::surfaceView()
{
    return static_cast<WidgetRenderer *>(renderer());
}

void AVMediaPlayer::setSurfaceView(QWidget * render)
{
    WidgetRenderer * widgetRender =static_cast<WidgetRenderer *>(render) ;
    setRenderer(widgetRender);
}

MediaPlayer::State AVMediaPlayer::videoState() const
{
    if(mediaStatus() == MediaStatus::BufferingMedia)
        return MediaPlayer::State::LoadingState;
    if(preparedState_)
        return MediaPlayer::State::PreparedState;
    if(state() == AVPlayer::State::PlayingState)
        return MediaPlayer::State::PlayingState;
    if(state() == AVPlayer::State::PausedState)
        return MediaPlayer::State::PausedState;
    if(state() == AVPlayer::State::StoppedState)
        return MediaPlayer::State::StoppedState;
    return MediaPlayer::UnknownStatus;
}

void AVMediaPlayer::setVideoState(MediaPlayer::State state)
{
    switch (state) {
    case MediaPlayer::State::PausedState:
        pause(true);
        break;
    case MediaPlayer::State::StoppedState:
        stop();
        break;
    case MediaPlayer::State::PlayingState:
        isPlaying()?pause(false):play();
        break;
    default:
        break;
    }
}

QSizeF AVMediaPlayer::videoSize() const
{
    return QSizeF(statistics().video_only.width,statistics().video_only.height);
}

bool AVMediaPlayer::autoPlay() const
{
    return autoPlay_;
}

void AVMediaPlayer::setAutoPlay(bool autoPlay)
{
    autoPlay_ = autoPlay;
    emit autoPlayChanged();
}

void AVMediaPlayer::setPosition(qint64 pos)
{
    if(isPlaying())
        AVPlayer::setPosition(pos);
    else
        AVPlayer::setStartPosition(pos);
    emit positionChanged();

}

qint64 AVMediaPlayer::position() const
{
    if(isPlaying())
        return AVPlayer::position();
    return AVPlayer::startPosition();
}

void AVMediaPlayer::applyVolume()
{
    AudioOutput *ao = audio();
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
