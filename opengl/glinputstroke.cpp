#include "glinputstroke.h"
#include "glstrokerenderer.h"

GLInputStroke::GLInputStroke(GLStrokeRenderer * glRenderer)
    : renderer_(glRenderer)
{

}

void GLInputStroke::setMaximun(const StrokePoint &max)
{
    scales_[0] = 2.0f / max[0];
    scales_[1] = 2.0f / max[1];
    scales_[2] = 4.0f / max[0];
}

void GLInputStroke::addPoint(StrokePoint const & point)
{
    points_[0] = point[0] * scales_[0] - 1.0f;
    points_[1] = 1.0f - point[1] * scales_[1];
    points_[2] = point[2] * scales_[2];
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
