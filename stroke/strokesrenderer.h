#ifndef STROKESRENDERER_H
#define STROKESRENDERER_H

#include "ShowBoard_global.h"
#include "core/lifeobject.h"

#include "strokepoint.h"

#include <QTimer>

class StrokesReader;

class SHOWBOARD_EXPORT StrokesRenderer : public LifeObject
{
    Q_OBJECT
public:
    explicit StrokesRenderer(StrokesReader* reader, QObject *parent = nullptr);

public:
    StrokesReader* reader() const { return reader_; }

public:
    bool start();

    void setPlayRate(float rate); // 0 for fastest, 1.0 for normal

    int time() const;

    void resume(); // start or resume

    void pause();

    void togglePlay(); // start/resume or pause

    bool isStarted() const { return timer_; }

    bool isPlaying() const { return timer_ && !paused_; }

    bool isPaused() const { return paused_; }

    void stop();

protected:
    virtual void setMaximun(StrokePoint const & max) = 0;

    virtual void startStroke(StrokePoint const & point) = 0;

    virtual void addPoint(StrokePoint const & point) = 0;

    virtual void endStroke() = 0;

    virtual void addNonStrokePoint(StrokePoint const & point) { (void) point; }

    virtual void startDynamic() {}

    virtual void finish() {}

protected:
    void seek(int time, int time2, int byte);

    int pointTime() const { return time_ * maximun_.t; }

private slots:
    void bump();

    void startAsync();

private:
    void addPoint2(StrokePoint const & point);

protected:
    StrokesReader* reader_;
    StrokePoint maximun_;
    bool dynamicStarted_ = false;
    bool strokeStarted_ = false;
    int strokeByte_ = 0; // stroke start byte position

private:
    StrokePoint point_; // last point
    bool pending_ = false;
    int time_ = 0; // last point time
    int byte_ = 0;

private:
    QTimer *timer_ = nullptr;
    float rate_ = 0;
    bool paused_ = false;
    /* this is real tick time
     *  if playing, this is start time
     *  if paused, this is paused stroke position
     */
    int startTime_ = 0; // in ticks
    int sleepTime_ = 0; // in point time
};

#endif // STROKESRENDERER_H
