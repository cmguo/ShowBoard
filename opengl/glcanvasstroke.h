#ifndef GLDYNAMICRENDERER_H
#define GLDYNAMICRENDERER_H

#include "stroke/strokerenderer.h"
#include "glinputstroke.h"

class GLStrokeRenderer;

class GLCanvasStroke : public StrokeRenderer, public GLInputStroke
{
public:
    GLCanvasStroke(StrokeReader* reader, GLStrokeRenderer * sr, QObject * parent = nullptr);

public:
    virtual void setMaximun(StrokePoint const & max) override;

    virtual void addPoint(StrokePoint const & point) override;

    virtual void endStroke() override;

    virtual void startDynamic() override;

private:
    GLStrokeRenderer * renderer_;
    float scales_[3];
    float points_[3];
};

#endif // GLDYNAMICRENDERER_H
