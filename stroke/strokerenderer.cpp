#include "strokerenderer.h"
#include "strokereader.h"

StrokeRenderer::StrokeRenderer(StrokeReader* reader, QObject *parent)
    : LifeObject(parent)
    , reader_(reader)
{
}

bool StrokeRenderer::start()
{
    StrokePoint point;
    if (!reader_->getMaximun(point))
        return false;
    setMaximun(point);
    while (reader_->read(point)) {
        addPoint2(point);
    }
    bool async = reader_->startAsyncRead([l = life(), this] (StrokePoint const & point) {
       addPoint2(point);
    });
    if (async)
        startDynamic();
    else
        finish();
    return true;
}

void StrokeRenderer::addPoint2(const StrokePoint &point)
{
    if (point[2]) {
        addPoint(point);
        strokeStarted_ = true;
    } else if (strokeStarted_) {
        endStroke();
        strokeStarted_ = false;
    }
}
