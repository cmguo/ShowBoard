#include "glinputstroke.h"
#include "glstrokerenderer.h"

GLInputStroke::GLInputStroke(GLStrokeRenderer * glRenderer)
    : renderer_(glRenderer)
{

}

void GLInputStroke::setMaximun(const StrokePoint &max)
{
    scales_[0] = 2.0f / max.x;
    scales_[1] = 2.0f / max.y;
    scales_[2] = 4.0f / max.p;
}

void GLInputStroke::addPoint(StrokePoint const & point)
{
    points_[0] = point.x * scales_[0] - 1.0f;
    points_[1] = 1.0f - point.y * scales_[1];
    points_[2] = point.p * scales_[2];
    push(points_);
    renderer_->Invalidate();
}

void GLInputStroke::endStroke()
{
    if (!finished()) {
        push();
        renderer_->Invalidate();
    }
}
