#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H

#include "core/control.h"

#include "widgetcontrol.h"

class MediaPlayer;
class SHOWBOARD_EXPORT VideoControl : public WidgetControl
{
    Q_OBJECT
    Q_PROPERTY(bool fullScreen READ isFullScreen WRITE fullScreen)

public:
    Q_INVOKABLE VideoControl(ResourceView *res);

    virtual ~VideoControl();

    bool isFullScreen() const;

    QObject *mediaPlayer() const;

public slots:

    void fullScreen(bool);

protected:

    virtual QWidget * createWidget(ControlView * parent) override;

    virtual void attached() override;

    virtual void detached() override;

private:
    MediaPlayer * player_;
    QWidget * fullScreenWidget_;
};

#endif // VIDEOCONTROL_H
