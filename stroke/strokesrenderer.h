#ifndef STROKESRENDERER_H
#define STROKESRENDERER_H

#include "ShowBoard_global.h"
#include "core/lifeobject.h"

#include "strokepoint.h"

class StrokesReader;

class SHOWBOARD_EXPORT StrokesRenderer : public LifeObject
{
    Q_OBJECT
public:
    explicit StrokesRenderer(StrokesReader* reader, QObject *parent = nullptr);

public:
    bool start(int count = -1);

    int time() const { return time_ * maximun_.t; }

protected:
    virtual void setMaximun(StrokePoint const & max) = 0;

    virtual void addPoint(StrokePoint const & point) = 0;

    virtual void endStroke() = 0;

    virtual void startDynamic() = 0;

    virtual void finish() = 0;

private slots:
    void start2(int count);

    void startAsync();

private:
    void addPoint2(StrokePoint const & point);

protected:
    StrokesReader* reader_;
    StrokePoint maximun_;
    bool dynamicStarted_ = false;
    bool strokeStarted_ = false;

private:
    ushort lastTime_ = 0;
    int time_ = 0;
};

#endif // STROKESRENDERER_H
