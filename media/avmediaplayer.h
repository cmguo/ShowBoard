#ifndef AVMEDIAPLAYER_H
#define AVMEDIAPLAYER_H

#include "mediaplayer.h"

#include <QtAV/AVPlayer.h>

#include <QtAVWidgets/VideoPreviewWidget.h>

using namespace QtAV;

class AVMediaPlayer : public MediaPlayer
{
    Q_OBJECT
    Q_PROPERTY(qint64 buffered READ bufferProgress NOTIFY bufferProgressChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChange)
public:
    AVMediaPlayer(QObject * parent =nullptr);

    virtual ~AVMediaPlayer() override;
    Q_INVOKABLE void togglePlayPause();

    Q_INVOKABLE void seek(qint64 value);

    Q_INVOKABLE void onTimeSliderHover(int pos, int value);

    Q_INVOKABLE void onTimeSliderLeave();


    qint64 position () const;

    void setPosition(qint64 pos);

    qreal bufferProgress() const;

    qint64 duration() const;

    bool isPlaying() const;

    qreal volume() const;

    void setVolume(qreal value);

    bool isMuted() const;

    void setMuted(bool m);

    bool isLoading() const;

    virtual void showNextFrame() override;

    virtual void setUrl(QString url) override;

    virtual QWidget *createRenderer() override;

    virtual void removeRenderer(QWidget *) override;

    virtual void pause() override;


Q_SIGNALS:
    void bufferProgressChanged(qreal);
    void sourceChanged();
    void durationChanged(qint64);
    void speedChanged(qreal speed);
    void positionChanged(qint64 position);
    void playingChanged();
    void volumeChanged();
    void mutedChanged();
    void fullScreenChanged(bool);
    void loadingChange();

private Q_SLOTS:
    void applyVolume();

private:
    AVPlayer * player_;
    VideoPreviewWidget *preview_;
};

#endif // AVMEDIAPLAYER_H
