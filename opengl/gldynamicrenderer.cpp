#include "gldynamicrenderer.h"
#include "glstrokerenderer.h"

GLDynamicRenderer::GLDynamicRenderer(GLStrokeRenderer * sr)
    : renderer_(sr)
{

}

void GLDynamicRenderer::addPoint(float point[3])
{
    if (point == nullptr)
    {
        if (!finished()) {
            push();
            renderer_->Invalidate();
        }
    }
    else
    {
        points_[0] = point[0] * 2.0f - 1.0f;
        points_[1] = point[1] * 2.0f - 1.0f;
        points_[2] = point[2] * 2.0f;
        push(points_);
        renderer_->Invalidate();
    }
}
