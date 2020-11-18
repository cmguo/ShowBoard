#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>


class MediaPlayer : public QObject{

    Q_OBJECT
public:
    MediaPlayer(QObject * parent);

    virtual ~MediaPlayer();

    virtual void showNextFrame();

    virtual void setUrl(QString url) = 0;

    virtual QWidget * createRenderer();

    virtual void pause();

    virtual void removeRenderer(QWidget *);

protected:
    qreal volume_;
    bool mute_;
};

#endif // MEDIAPLAYER_H
