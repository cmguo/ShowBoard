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

    float rate() const { return realRate_; }

    void setRate(float rate); // 0 for fastest, 1.0 for normal

    int maxGap() const { return maxGap_; }

    void setMaxGap(int time); // adjust large gaps

    int time() const;

    int duration() const;

    void resume(); // start or resume

    void pause();

    void togglePlay(); // start/resume or pause

    bool isStarted() const { return timer_; }

    bool isPlaying() const { return timer_ && !paused_; }

    bool isPaused() const { return paused_; }

    void stop();

signals:
    void positionChanged();

protected:
    virtual void setMaximun(StrokePoint const & max) = 0;

    virtual void startStroke(StrokePoint const & point) = 0;

    virtual void addPoint(StrokePoint const & point) = 0;

    virtual void endStroke() = 0;

    virtual void addNonStrokePoint(StrokePoint const & point) { (void) point; }

    virtual void enterFastMode() {}

    virtual void leaveFastMode() {}

    virtual void finish() {}

protected:
    // time2 is adjust to the time of next point
    void seek(int time, int time2, int byte, bool inStroke);

    int t() const { return time_; }

    int b() const { return byte_; }

private slots:
    void bump();

    void startAsync();

    void stopAsync();

private:
    void addPoint2(StrokePoint const & point);

protected:
    StrokesReader* reader_;
    StrokePoint maximun_;
    int interval_ = 20; // in point time

protected:
    bool strokeStarted_ = false;
    bool fastMode_ = false;
    int strokeTime_ = 0; // stroke start time position
    int strokeByte_ = 0; // stroke start byte position

private:
    StrokePoint point_; // last point
    bool pending_ = false;
    int time_ = 0; // last point time
    int byte_ = 0;
    int maxTime_ = 0; // update when seek
    int maxByte_ = 0; // update when seek
    int notifyTime_ = 0;
    bool maxInStroke_ = false;
    bool dynamicStarted_ = false;
    float realRate_ = 0;
    int maxGap_ = 0;

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
    int seekTime_ = 0; // in point time
};

#endif // STROKESRENDERER_H
