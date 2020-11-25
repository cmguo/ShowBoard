#ifndef VIDEOCONTROL_H
#define VIDEOCONTROL_H

#include "core/control.h"

#include "widgetcontrol.h"

class MediaPlayerBridge;
class SHOWBOARD_EXPORT VideoControl : public WidgetControl
{
    Q_OBJECT
    Q_PROPERTY(bool fullScreen READ isFullScreen WRITE fullScreen NOTIFY fullScreenChanged)

public:
    Q_INVOKABLE VideoControl(ResourceView *res);

    virtual ~VideoControl() override;

    bool isFullScreen() const;

    QObject *mediaPlayer() const;

Q_SIGNALS:
    void fullScreenChanged();

public slots:
    void fullScreen(bool);

//    void play();

//    void seek();

//    void stop();

protected:

    virtual QWidget * createWidget(ControlView * parent) override;

    virtual void attached() override;

    virtual void detached() override;

private slots:
    void loaded();

private:
    MediaPlayerBridge * playerBridge_;
    QObject * player_;
    QWidget * fullScreenWidget_;
};

#endif // VIDEOCONTROL_H
