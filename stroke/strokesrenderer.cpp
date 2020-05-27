#include "strokesrenderer.h"
#include "strokesreader.h"

#include <QElapsedTimer>

StrokesRenderer::StrokesRenderer(StrokesReader* reader, QObject *parent)
    : LifeObject(parent)
    , reader_(reader)
    , point_(StrokePoint::EndStorke)
{
    reader->setParent(this);
}

static QElapsedTimer startTick()
{
    QElapsedTimer t;
    t.start();
    return t;
}

static int tickCount()
{
    static QElapsedTimer t = startTick();
    return static_cast<int>(t.elapsed());
}

bool StrokesRenderer::start()
{
    if (!reader_->getMaximun(maximun_))
        return false;
    setMaximun(maximun_);
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &StrokesRenderer::bump);
    startTime_ = tickCount();
    bump();
    return true;
}

void StrokesRenderer::setPlayRate(float rate)
{
    int t = time();
    rate_ = rate;
    if (isPlaying()) {
        if (rate_ > 0)
            startTime_ = tickCount() - static_cast<int>(t / rate_);
    }
}

int StrokesRenderer::time() const
{
    if (rate_ > 0)
        return static_cast<int>((tickCount() - startTime_) * rate_);
    else
        return pointTime();
}

void StrokesRenderer::resume()
{
    if (paused_) {
        if (rate_ > 0)
            startTime_ = tickCount() - static_cast<int>(startTime_ / rate_);
        paused_ = false;
        sleepTime_ = time_;
        bump();
    }
}

void StrokesRenderer::pause()
{
    if (!paused_) {
        if (rate_ > 0)
            startTime_ = static_cast<int>((tickCount() - startTime_) * rate_);
        paused_ = true;
        timer_->stop();
    }
}

void StrokesRenderer::togglePlay()
{
    if (!isStarted()) {
        start();
    } else if (paused_) {
        resume();
    } else {
        pause();
    }
}

void StrokesRenderer::seek(int time, int time2, int byte)
{
    reader_->stopAsyncRead();
    reader_->seek(byte);
    point_.t -= time_ - time2;
    time_ = time2;
    pending_ = false;
    if (paused_) {
        startTime_ = time;
        return;
    }
    pause();
    if (rate_ > 0)
        startTime_ = tickCount() - static_cast<int>(time / rate_);
    resume();
}

void StrokesRenderer::stop()
{
    if (timer_ == nullptr)
        return;
    if (byte_)
        reader_->seek(0);
    point_ = StrokePoint::EndStorke;
    time_ = 0;
    byte_ = 0;
    paused_ = false;
    startTime_ = 0;
    sleepTime_ = 0;
    delete timer_;
    timer_ = nullptr;
}

void StrokesRenderer::bump()
{
    if (pending_) {
        addPoint2(point_);
        pending_ = false;
    }
    StrokePoint point;
    if (rate_ > 0)
        sleepTime_ += static_cast<int>(20 / rate_);
    else
        sleepTime_ += 20;
    while (reader_->read(point, byte_)) {
        if (maximun_.t) {
            if (time_ > 0) {
                time_ += point.t - point_.t;
            } else if (point_.t != 0) {
                time_ = point.t - point_.t;
            }
        } else {
            time_ += 10;
        }
        point_ = point;
        if (time_ >= sleepTime_) {
            if (rate_ > 0)
                timer_->start(static_cast<int>(time_ / rate_) - tickCount());
            else
                timer_->start(); // restart when idle
            sleepTime_ = time_;
            pending_ = true;
            return;
        }
        addPoint2(point);
    }
    startAsync();
}

void StrokesRenderer::startAsync()
{
    bool async = reader_->startAsyncRead([l = life(), this] (StrokePoint const & point, int bytePos) {
        if (!l.isNull()) {
            byte_ = bytePos;
            addPoint2(point);
        }
    });
    if (async) {
        dynamicStarted_ = true;
        startDynamic();
    } else {
        if (strokeStarted_) {
            endStroke();
            strokeStarted_ = false;
        }
        finish();
    }
}

void StrokesRenderer::addPoint2(const StrokePoint &point)
{
    if (!point.s) {
        if (!strokeStarted_) {
            strokeStarted_ = true;
            strokeByte_ = byte_;
            startStroke(point);
        } else {
            addPoint(point);
        }
    } else {
        if (strokeStarted_) {
            endStroke();
            strokeStarted_ = false;
        } else {
            addNonStrokePoint(point);
        }
    }
}
