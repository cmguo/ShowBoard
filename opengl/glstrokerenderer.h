#ifndef GLSTROKERENDERER_H
#define GLSTROKERENDERER_H

#include "glstrokeshaders.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QElapsedTimer>

class GLStroke;

class GLStrokeRenderer : public GLStrokeShaders
{
public:
    typedef std::function<void()> Action;

public:
    GLStrokeRenderer(Action invalidate);

public:
    class Canvas;

    void SetBackground(QString const & path);

    void SetBackground(QPixmap bitmap);

    Canvas * CreateCanvas(GLStroke * stroke, QRectF rect, bool bg = true);

    void InitDraw();

    void Invalidate();

    void SetMultiSample(bool v);

    void AdjustScale(float diff);

    void Refresh();

    void OnSizeChanged(int w, int h);

    void Draw();

    void DoPrevDraw(Action action);

    void DoPostDraw(Action action);

private:
    void DrawBackgroud(Canvas & c);

    void DrawStroke(Canvas & c, bool refresh);

private:
    QOpenGLShaderProgram mStrokeProgram;
    QOpenGLShaderProgram mStrokeProgram2;

    QOpenGLShaderProgram mBackgroundProgram;
    QOpenGLVertexArrayObject mBackgroundArray;
    QOpenGLBuffer mBackgroundBuffer; // Target.ArrayBuffer

    QOpenGLTexture mBackgroundTexture;

    QList<Action> prevDraw;
    QList<Action> postDraw;

    Action invalidate;
    bool refresh = false;
    float scale = 1.0f;

    int mWidth = 0;
    int mHeight = 0;
    QList<Canvas *> mCanvases;

private:
    QElapsedTimer mTimer;
    long mDrawCount = 0;
    long mDrawCount2 = 0;
    long mNextTick = 0;
    long mSumTick = 0;
};

#endif // GLSTROKERENDERER_H
