#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H

#include "core/control.h"

class QMediaPlayer;

class VideoControl : public Control
{
    Q_OBJECT

    Q_PROPERTY(qreal playRate READ playRate WRITE setPlayRate)

public:
    Q_INVOKABLE VideoControl(ResourceView *res);

public:
    qreal playRate() const;

    void setPlayRate(qreal v);

public slots:
    void play();

    void seek();

    void fullScreen();

    void stop();

protected:
    virtual ControlView * create(ControlView * parent) override;

    virtual void attached() override;

    virtual void detached() override;

    virtual void resize(const QSizeF &size) override;

    virtual void updateToolButton(ToolButton * button) override;

private:
    QMediaPlayer * player_;
    QWidget * fullscreen_;
};

#endif // VIDEOCONTROL_H
