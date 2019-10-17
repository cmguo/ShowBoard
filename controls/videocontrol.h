#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H

#include "control.h"

class QMediaPlayer;

class VideoControl : public Control
{
    Q_OBJECT
public:
    Q_INVOKABLE VideoControl(ResourceView *res);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual void sizeChanged(QSizeF size) override;

    virtual void detach() override;

private:
    QMediaPlayer * player_;
};

#endif // VIDEOCONTROL_H
