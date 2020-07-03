#include "strokesrenderer.h"
#include "strokesreader.h"

#include <QElapsedTimer>
#include <QDebug>

static constexpr int INTERVAL = 20; // ms (tick)

StrokesRenderer::StrokesRenderer(StrokesReader* reader, QObject *parent)
    : LifeObject(parent)
    , reader_(reader)
    , maximun_(StrokePoint::EndStorke)
    , point_(StrokePoint::EndStorke)
{
    reader->setParent(this);
    connect(reader, &StrokesReader::asyncFinished, this, &StrokesRenderer::asyncFinished);
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
    if (timer_)
        return false;
    if (!reader_->getMaximun(maximun_))
        return false;
    if (maximun_.t) {
        interval_ = INTERVAL < maximun_.t ? 1 : INTERVAL / maximun_.t;
        if (rate_ > 0)
            interval_ = interval_ < rate_
                    ? interval_ : static_cast<int>(interval_ / rate_);
        rate_ = rate_ / maximun_.t;
    }
    setMaximun(maximun_);
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &StrokesRenderer::bump);
    startTime_ = tickCount();
    bump();
    return true;
}

void StrokesRenderer::setRate(float rate)
{
    int t = time();
    int mt = maximun_.t ? maximun_.t : 1;
    realRate_ = rate;
    rate_ = rate / mt;
    interval_ = INTERVAL < mt ? 1 : INTERVAL / mt;
    if (rate > 0)
        interval_ = static_cast<int>(interval_ * rate);
    if (isPlaying()) {
        if (rate > 0)
            startTime_ = tickCount() - static_cast<int>(t / rate);
        pause();
        resume();
    }
}

void StrokesRenderer::setMaxGap(int time)
{
    maxGap_ = time;
}

int StrokesRenderer::time() const
{
    if (rate_ > 0)
        return static_cast<int>((tickCount() - startTime_) * realRate_);
    else
        return maximun_.t ? time_ * maximun_.t : time_;
}

int StrokesRenderer::duration() const
{
    int tm = qMax(time_, maxTime_);
    return maximun_.t ? tm * maximun_.t : tm;
}

void StrokesRenderer::resume()
{
    if (paused_) {
        if (rate_ > 0)
            startTime_ = tickCount() - static_cast<int>(startTime_ / rate_);
        //qDebug() << "resume" << startTime_;
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
        //qDebug() << "pause" << startTime_;
        paused_ = true;
        if (time_ >= seekTime_)
            timer_->stop();
        stopAsync();
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

void StrokesRenderer::seek(int time, int time2, int byte, bool inStroke)
{
    qDebug() << "seek" << time << time2 << byte <<inStroke;
    if (byte >= 0) {
        stopAsync();
        // save max position
        byte_ = reader_->bytePos();
        if (byte_ > maxByte_) {
            maxByte_ = byte_;
            maxTime_ = time_; // previous point time
            maxInStroke_ = strokeStarted_;
        }
        // seek to end (max) or after end
        if (time < 0 || time > maxTime_) {
            if (time < 0)
                time = maxTime_;
            time2 = maxTime_;
            byte = maxByte_;
            inStroke = maxInStroke_;
        }
        qDebug() << "seek" << time << time2 << byte <<inStroke;
        reader_->seek(byte);
        byte_ = byte;
        // after adjust, next point read will has 0 diff
        // also can be previous point time
        point_.t -= time_ - time2;
        time_ = time2;
        pending_ = false;
        strokeStarted_ = inStroke;
    }
    seekTime_ = time;
    notifyTime_ = time;
    finished_ = false;
    if (paused_) {
        startTime_ = time_;
        if (time_ < seekTime_) {
            bump();
        }
        return;
    }
    pause();
    if (rate_ > 0)
        startTime_ = time; // use real seek time
    resume();
}

void StrokesRenderer::stop()
{
    if (timer_ == nullptr)
        return;
    pause();
    if (byte_)
        reader_->seek(0);
    point_ = StrokePoint::EndStorke;
    time_ = 0;
    byte_ = 0;
    strokeByte_ = 0;
    strokeTime_ = 0;
    strokeStarted_ = false;
    paused_ = false;
    fastMode_ = false;
    finished_ = false;
    startTime_ = 0;
    sleepTime_ = 0;
    seekTime_ = 0;
    delete timer_;
    timer_ = nullptr;
}

/*
 * pending_
 * sleepTime_ (only read/write here)
 * seekTime_
 * time_
 * paused_
 */
void StrokesRenderer::bump()
{
    if (time_ >= notifyTime_ && rate_ > 0 && time_ >= seekTime_) {
        notifyTime_ += (maximun_.t ? 1000 / maximun_.t : 1000);
        emit positionChanged();
        int d = static_cast<int>(time_ / rate_) + startTime_ - tickCount();
        if (d > 1000) {
            qDebug() << "bump gap continue" << d;
            d = 1000;
        }
        timer_->start(d < 0 ? 0 : d);
        return;
    }
    if (pending_) {
        addPoint2(point_);
        pending_ = false;
    }
    if (finished_)
        return;
    StrokePoint point;
    bool nonFast = rate_ > 0 && time_ >= seekTime_;
    sleepTime_ += interval_;
    if (fastMode_ == nonFast) {
        fastMode_ = !nonFast;
        fastMode_ ? enterFastMode() : leaveFastMode();
        emit positionChanged();
    }
    while (reader_->read(point, byte_)) {
        //qDebug() << "bump" << byte_ << point.t << point.x << point.y;
        if (maximun_.t) {
            if (time_ > 0) {
                time_ += (point.t - point_.t) & 0xffff;
                assert(time_ > 0);
            } else if (point_.t != 0) {
                time_ = (point.t - point_.t) & 0xffff;
            }
        } else {
            time_ += 10;
        }
        point_ = point;
        //assert(time_ < 1000);
        //qDebug() << "bump" << time_;
        if (time_ >= sleepTime_) {
            //qDebug() << "bump" << time_ << sleepTime_;
            if (paused_ && time_ >= seekTime_) {
                emit positionChanged();
                pending_ = true;
                return;
            }
            if (nonFast) {
                int d = static_cast<int>(time_ / rate_) + startTime_ - tickCount();
                if (maxGap_ && d > maxGap_) {
                    qDebug() << "bump skip gap" << time_ << sleepTime_ << startTime_ << tickCount() << d;
                    qDebug() << "bump skip gap" << d;
                    startTime_ -= d - maxGap_;
                    d = maxGap_;
                }
                if (d > 1000) {
                    qDebug() << "bump gap" << d;
                    d = 1000;
                }
                timer_->start(d < 0 ? 0 : d);
            } else { // rate_ == 0 || time_ < seekTime_, but new time_ may > seekTime_
                timer_->start(0); // restart when idle
            }
            if (time_ >= notifyTime_) {
                notifyTime_ += (maximun_.t ? 1000 / maximun_.t : 1000);
                emit positionChanged();
            }
            sleepTime_ = time_;
            pending_ = true;
            return;
        }
        addPoint2(point);
    }
    seekTime_ = time_; // can't fast seek any more
    if (!paused_)
        startAsync();
}

void StrokesRenderer::startAsync()
{
    qDebug() << "startAsync" << byte_;
    bool async = reader_->startAsyncRead([l = life(), this] (StrokePoint const & point, int bytePos) {
        if (!l.isNull()) {
            byte_ = bytePos;
            //qDebug() << "async" << byte_ << point.t << point.x << point.y;
            if (maximun_.t) {
                if (time_ > 0) {
                    time_ += (point.t - point_.t) & 0xffff;
                } else if (point_.t != 0) {
                    time_ = (point.t - point_.t) & 0xffff;
                }
            } else {
                time_ += 10;
            }
            //qDebug() << "async" << byte_ << time_;
            point_ = point;
            addPoint2(point);
        }
    });
    if (async) {
        asyncStarted_ = true;
        if (fastMode_) {
            fastMode_ = false;
            leaveFastMode();
        }
        emit positionChanged();
    } else {
        finish();
    }
}

void StrokesRenderer::stopAsync()
{
    if (asyncStarted_) {
        // reader should keep position valid in async mode
        reader_->stopAsyncRead();
        asyncStarted_ = false;
    }
}

void StrokesRenderer::asyncFinished()
{
    if (asyncStarted_) {
        stopAsync();
        finish();
    }
}

void StrokesRenderer::finish()
{
    if (strokeStarted_) {
        endStroke();
        strokeStarted_ = false;
    }
    emit positionChanged();
    if (fastMode_) {
        fastMode_ = false;
        leaveFastMode();
    }
    finished_ = true;
    qDebug() << "finish" << time_ << byte_;
    onFinish();
    emit finished();
}

void StrokesRenderer::addPoint2(const StrokePoint &point)
{
    if (!point.s) {
        if (!strokeStarted_) {
            strokeStarted_ = true;
            strokeTime_ = time_;
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
