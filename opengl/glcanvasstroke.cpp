#include "glcanvasstroke.h"
#include "glstrokerenderer.h"

GLCanvasStroke::GLCanvasStroke(StrokesReader* reader, GLStrokeRenderer * sr, QObject * parent)
    : StrokesRenderer(reader, parent)
    , GLInputStroke(sr)
    , renderer_(sr)
{
}

void GLCanvasStroke::setMaximun(const StrokePoint &max)
{
    GLInputStroke::setMaximun(max);
}

void GLCanvasStroke::addPoint(StrokePoint const & point)
{
    GLInputStroke::addPoint(point);
}

void GLCanvasStroke::endStroke()
{
    GLInputStroke::endStroke();
}

void GLCanvasStroke::startDynamic()
{
}
