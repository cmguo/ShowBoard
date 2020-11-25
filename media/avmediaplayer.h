#ifndef AVMEDIAPLAYER_H
#define AVMEDIAPLAYER_H

#include "mediaplayer.h"
#include <QtAV/AVPlayer.h>
#include <QtAVWidgets/VideoPreviewWidget.h>

using namespace QtAV;

class AVMediaPlayer : public AVPlayer
{
    Q_OBJECT
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(MediaPlayer::State videoState READ videoState WRITE setVideoState NOTIFY videoStateChanged)
    Q_PROPERTY(QWidget* surfaceView READ surfaceView WRITE setSurfaceView NOTIFY surfaceViewChange)
    Q_PROPERTY(QSizeF videoSize READ videoSize NOTIFY videoSizeChanged)
    Q_PROPERTY(bool autoPlay READ autoPlay WRITE setAutoPlay NOTIFY autoPlayChanged)
public:

    Q_INVOKABLE AVMediaPlayer(QObject * parent =nullptr);

    virtual ~AVMediaPlayer() override;

    qreal volume() const;

    void setVolume(qreal value);

    bool isMuted() const;

    void setMuted(bool m);

    QString source() const;

    void setSource(const QString source);

    QWidget* surfaceView();

    void setSurfaceView(QWidget *);

    MediaPlayer::State videoState()const;

    void setVideoState(MediaPlayer::State state);

    QSizeF videoSize() const;

    bool autoPlay()const;

    void setAutoPlay(bool autoPlay);

    void setPosition(qint64 pos);

    qint64 position() const;

Q_SIGNALS:
    void volumeChanged();
    void mutedChanged();
    void surfaceViewChange();
    void videoStateChanged();
    void videoSizeChanged();
    void autoPlayChanged();
    void positionChanged();

private Q_SLOTS:
    void applyVolume();

private:
    qreal volume_;
    bool mute_;
    bool autoPlay_;
    bool preparedState_;
    bool loaded_;
    VideoPreviewWidget *preview_;
};

#endif // AVMEDIAPLAYER_H
