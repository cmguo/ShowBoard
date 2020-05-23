#ifndef GLINPUTSTROKE_H
#define GLINPUTSTROKE_H

#include "glstroke.h"
#include "stroke/strokepoint.h"

class GLStrokeRenderer;

class GLInputStroke : public GLStroke
{
public:
    GLInputStroke(GLStrokeRenderer * glRenderer);

protected:
    void setMaximun(StrokePoint const & max);

    void addPoint(StrokePoint const & point);

    void endStroke();

private:
    GLStrokeRenderer * renderer_;
    float scales_[3];
    float points_[3];
};

#endif // GLINPUTSTROKE_H
