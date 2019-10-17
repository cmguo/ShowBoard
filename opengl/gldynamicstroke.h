#ifndef GLDYNAMICSTROKE_H
#define GLDYNAMICSTROKE_H

#include "resources/stroke.h"
#include "glstroke.h"

class GLStrokeRenderer;

class GLDynamicStroke : public GLStroke, IDynamicStroke
{
public:
    GLDynamicStroke(GLStrokeRenderer * sr);

public:
    void addPoint(float point[3]);

private:
    GLStrokeRenderer * renderer_;
    float points_[3];
};

#endif // GLDYNAMICSTROKE_H
