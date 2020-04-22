#include "gldynamicrenderer.h"
#include "glstrokerenderer.h"

GLDynamicRenderer::GLDynamicRenderer(GLStrokeRenderer * sr)
    : renderer_(sr)
{
}

void GLDynamicRenderer::setMaximun(const stroke_point_t &max)
{
    scales_[0] = 2.0f / max[0];
    scales_[1] = 2.0f / max[1];
    scales_[2] = 4.0f / max[0];
}

void GLDynamicRenderer::addPoint(stroke_point_t const & point)
{
    if (point[2] == 0)
    {
        if (!finished()) {
            push();
            renderer_->Invalidate();
        }
    }
    else
    {
        points_[0] = point[0] * scales_[0] - 1.0f;
        points_[1] = 1.0f - point[1] * scales_[1];
        points_[2] = point[2] * scales_[2];
        push(points_);
        renderer_->Invalidate();
    }
}
