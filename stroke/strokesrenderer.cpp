#include "strokesrenderer.h"
#include "strokesreader.h"

StrokesRenderer::StrokesRenderer(StrokesReader* reader, QObject *parent)
    : LifeObject(parent)
    , reader_(reader)
{
    reader->setParent(this);
}

bool StrokesRenderer::start(int count)
{
    StrokePoint point;
    if (!reader_->getMaximun(point))
        return false;
    setMaximun(point);
    if (count > 0) {
        start2(count);
    } else {
        while (reader_->read(point)) {
            addPoint2(point);
        }
        startAsync();
    }
    return true;
}

void StrokesRenderer::start2(int count)
{
    StrokePoint point;
    int n = count;
    while (n && reader_->read(point)) {
        addPoint2(point);
        --n;
    }
    if (n == 0) {
        metaObject()->invokeMethod(this, "restart", Qt::QueuedConnection, Q_ARG(int,count));
    } else {
        startAsync();
    }
}

void StrokesRenderer::startAsync()
{
    bool async = reader_->startAsyncRead([l = life(), this] (StrokePoint const & point) {
        if (!l.isNull())
            addPoint2(point);
    });
    dynamicStarted_ = true;
    if (async)
        startDynamic();
    else
        finish();
}

void StrokesRenderer::addPoint2(const StrokePoint &point)
{
    if (!point.s) {
        addPoint(point);
        strokeStarted_ = true;
    } else if (strokeStarted_) {
        endStroke();
        strokeStarted_ = false;
    }
}
