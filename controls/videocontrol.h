#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H

#include "core/control.h"

class QMediaPlayer;

class VideoControl : public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE VideoControl(ResourceView *res);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void detached() override;

private:
    QMediaPlayer * player_;
};

#endif // VIDEOCONTROL_H
