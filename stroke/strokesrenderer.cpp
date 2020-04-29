#include "strokesrenderer.h"
#include "strokesreader.h"

StrokesRenderer::StrokesRenderer(StrokesReader* reader, QObject *parent)
    : LifeObject(parent)
    , reader_(reader)
{
    reader->setParent(this);
}

bool StrokesRenderer::start()
{
    StrokePoint point;
    if (!reader_->getMaximun(point))
        return false;
    setMaximun(point);
    while (reader_->read(point)) {
        addPoint2(point);
    }
    bool async = reader_->startAsyncRead([l = life(), this] (StrokePoint const & point) {
        if (!l.isNull())
            addPoint2(point);
    });
    dynamicStarted_ = true;
    if (async)
        startDynamic();
    else
        finish();
    return true;
}

void StrokesRenderer::addPoint2(const StrokePoint &point)
{
    if (point[2]) {
        addPoint(point);
        strokeStarted_ = true;
    } else if (strokeStarted_) {
        endStroke();
        strokeStarted_ = false;
    }
}
