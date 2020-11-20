#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>


class MediaPlayer:public QObject
{
    Q_OBJECT
    Q_ENUMS(State)
public:
    enum State {
        UnknownStatus = 0,
        StoppedState,
        PlayingState,
        PausedState,
        LoadingState,
        PreparedState
    };
public:
    MediaPlayer(QObject *parent = nullptr);
};

#endif // MEDIAPLAYER_H
