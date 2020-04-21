#ifndef GLDYNAMICRENDERER_H
#define GLDYNAMICRENDERER_H

#include "resources/strokes.h"
#include "glstroke.h"

class GLStrokeRenderer;

class GLDynamicRenderer : public GLStroke, IDynamicRenderer
{
public:
    GLDynamicRenderer(GLStrokeRenderer * sr);

public:
    void addPoint(float point[3]);

private:
    GLStrokeRenderer * renderer_;
    float points_[3];
};

#endif // GLDYNAMICRENDERER_H
