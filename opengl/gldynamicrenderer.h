#ifndef GLDYNAMICRENDERER_H
#define GLDYNAMICRENDERER_H

#include "resources/strokes.h"
#include "glstroke.h"

class GLStrokeRenderer;

class GLDynamicRenderer : public IDynamicRenderer, public GLStroke
{
public:
    GLDynamicRenderer(GLStrokeRenderer * sr);

public:
    virtual void setMaximun(stroke_point_t const & max) override;

    virtual void addPoint(stroke_point_t const & point) override;

private:
    GLStrokeRenderer * renderer_;
    float scales_[3];
    float points_[3];
};

#endif // GLDYNAMICRENDERER_H
