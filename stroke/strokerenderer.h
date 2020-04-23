#ifndef STROKERENDERER_H
#define STROKERENDERER_H

#include "ShowBoard_global.h"
#include "core/lifeobject.h"

#include "strokepoint.h"

class StrokeReader;

class SHOWBOARD_EXPORT StrokeRenderer : public LifeObject
{
    Q_OBJECT
public:
    explicit StrokeRenderer(StrokeReader* reader, QObject *parent = nullptr);

public:
    bool start();

protected:
    virtual void setMaximun(StrokePoint const & max) = 0;

    virtual void addPoint(StrokePoint const & point) = 0;

    virtual void endStroke() = 0;

    virtual void startDynamic() = 0;

    virtual void finish() = 0;

private:
    void addPoint2(StrokePoint const & point);

protected:
    StrokeReader* reader_;
    bool strokeStarted_ = false;
};

#endif // STROKERENDERER_H
