#include "glstrokerenderer.h"

#include "glstroke.h"

#include <QTime>
#include <QPixmap>

GLStrokeRenderer::GLStrokeRenderer(Action invalidate)
    : mBackgroundBuffer(QOpenGLBuffer::VertexBuffer)
    , mBackgroundTexture(QOpenGLTexture::Target2D)
    , invalidate(invalidate)
{

}

class GLStrokeRenderer::Canvas
{
public:
    GLStroke * mStroke;
    QRectF mRect;
    QRect mBound;
    bool mBackgroud;
    ushort mPosition;

public:
    Canvas(GLStroke * stroke, QRectF const & rect, bool bg)
    {
        mStroke = stroke;
        mRect = rect;
        mBackgroud = bg;
    }

    void SetGLSize(int width, int height)
    {
        QRectF r = mRect.adjusted(1.0, 1.0, 1.0, 1.0);
        qreal w = width / 2;
        qreal h = height / 2;
        mBound = QRectF(r.x() * w, r.y() * h, r.width() * w, r.height() * h).toRect();
    }
};

void GLStrokeRenderer::SetBackground(QString const & path)
{
    (void)path;
    //mBackgroundTexture.QOpenGLTexture(QImage(path));
}

void GLStrokeRenderer::SetBackground(QPixmap bitmap)
{
    (void)bitmap;
    //mBackgroundTexture = GLUtils::CreateTexture(bitmap);
}

GLStrokeRenderer::Canvas * GLStrokeRenderer::CreateCanvas(
        GLStroke * stroke, QRectF rect, bool bg)
{
    Canvas * c = new Canvas(stroke, rect, bg);
    if (mWidth > 0 && mHeight > 0)
        c->SetGLSize(mWidth, mHeight);
    mCanvases.push_back(c);
    invalidate();
    return c;
}

void GLStrokeRenderer::InitDraw()
{
    initGL();
    glGetError();

    mStrokeProgram.create();
    new QOpenGLShader(QOpenGLShader::Vertex);
    mStrokeProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, STROKE_VERTEX_SHADER);
    mStrokeProgram.addShaderFromSourceCode(QOpenGLShader::Geometry, STROKE_GEOMETRY_SHADER2);
    mStrokeProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, STROKE_FRAGMENT_SHADER);
    mStrokeProgram.link();
    assertGL();

    mStrokeProgram.bind();
    assertGL();
    mStrokeProgram.setUniformValue(UNIFORM_COLOR, 0.0f, 0.0f, 1.0f, 1.0f);
    assertGL();
    mStrokeProgram.setUniformValue(UNIFORM_SCALE, scale);
    assertGL();

    //mStrokeProgram2 = GLUtils::CreateProgram(STROKE_VERTEX_SHADER, null, STROKE_FRAGMENT_SHADER);
    //assertGL();
    //mStrokeProgram2.bind();
    //assertGL();
    //glUniform4(UNIFORM_COLOR, 0f, 0f, 1f, 1f);
    //assertGL();

    mBackgroundProgram.create();
    mBackgroundProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, BACKGROUND_VERTEX_SHADER);
    mBackgroundProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, BACKGROUND_FRAGMENT_SHADER);
    mBackgroundProgram.link();
    mBackgroundArray.create();
    assertGL();
    mBackgroundBuffer.create();
    mBackgroundBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

    mBackgroundArray.bind();
    assertGL();
    mBackgroundBuffer.bind();
    assertGL();
    mBackgroundBuffer.allocate(sVertexData, sizeof(sVertexData));
    assertGL();
    mBackgroundProgram.setAttributeBuffer(VERTEX_POINT, GL_FLOAT, 0, 2, 0);
    assertGL();
    mBackgroundProgram.enableAttributeArray(VERTEX_POINT);
    assertGL();
    mBackgroundProgram.setAttributeBuffer(VERTEX_TEXTURE, GL_FLOAT, sizeof(sVertexData) / 2, 2, 0);
    assertGL();
    mBackgroundProgram.enableAttributeArray(VERTEX_TEXTURE);
    assertGL();

    if (mBackgroundTexture.isCreated())
        mBackgroundTexture.bind(0);
    assertGL();
    mBackgroundArray.release();
    assertGL();

    mBackgroundProgram.bind();
    assertGL();
    mBackgroundProgram.setUniformValue(UNIFORM_TEXTURE, 0);
    assertGL();

    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    assertGL();
    glClearColor(0, 0, 0, 0);
    assertGL();
    glEnable(GL_MULTISAMPLE);
    assertGL();

    mTimer.start();
    mNextTick = 1000;
}

void GLStrokeRenderer::Invalidate()
{
    invalidate();
}

void GLStrokeRenderer::AdjustScale(float diff)
{
    DoPrevDraw([this, diff]() {
        mStrokeProgram.bind();
        scale = diff < 0.001f ? 1.0f : scale + diff;
        mStrokeProgram.setUniformValue(UNIFORM_SCALE, scale);
    });
}

void GLStrokeRenderer::Refresh()
{
    refresh = true;
    Invalidate();
}

void GLStrokeRenderer::OnSizeChanged(int w, int h)
{
    mWidth = w;
    mHeight = h;
    for (Canvas * c : mCanvases)
    {
        c->SetGLSize(w, h);
    }
    refresh = true;
    invalidate();
}

void GLStrokeRenderer::Draw()
{
    ++mDrawCount;
    //assertGL();
    glGetError();
    while (!prevDraw.empty()) {
        Action action = prevDraw[0];
        prevDraw.pop_front();
        action();
    }
    auto tick = mTimer.elapsed();
    if (tick >= mNextTick) {
        double fps = static_cast<double>(mDrawCount - mDrawCount2) / static_cast<double>(tick - mNextTick + 1000) * 1000.0;
        mDrawCount2 = mDrawCount;
        mNextTick += 1000;
        qDebug() << "FPS:" << fps << ", Tick:" << mSumTick;
        mSumTick = 0;
    }
    if (refresh)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // drawBackgroud
    if (refresh && mBackgroundTexture.isCreated()) {
        mBackgroundProgram.bind();
        mBackgroundArray.bind();
        for (Canvas * c : mCanvases)
        {
            if (!c->mBackgroud)
                continue;
            glViewport(c->mBound.x(), c->mBound.y(), c->mBound.width(), c->mBound.height());
            assertGL();
            DrawBackgroud(*c);
        }
        mBackgroundArray.release();
    }
    // drawStroke
    mStrokeProgram.bind();
    for (Canvas * c : mCanvases)
    {
        glViewport(c->mBound.x(), c->mBound.y(), c->mBound.width(), c->mBound.height());
        mStrokeProgram.setUniformValue(
                    UNIFORM_PIXEL_SIZE, 2.0f / c->mBound.width(), 2.0f / c->mBound.height());
        //assertGL();
        DrawStroke(*c, refresh);
        //mStrokeProgram2.bind();
    }
    refresh = false;
    mSumTick += mTimer.elapsed() - tick;
    //*/
    while (!postDraw.empty())
    {
        Action action = postDraw[0];
        postDraw.pop_front();
        action();
    }
}

void GLStrokeRenderer::DoPrevDraw(Action action)
{
    prevDraw.push_back(action);
    refresh = true;
    invalidate();
}

void GLStrokeRenderer::DoPostDraw(Action action)
{
    postDraw.push_back(action);
    refresh = true;
    invalidate();
}

void GLStrokeRenderer::DrawBackgroud(Canvas & c)
{
    (void)c;
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void GLStrokeRenderer::DrawStroke(Canvas & c, bool refresh)
{
    c.mPosition += 4;
    if (c.mPosition >= c.mStroke->total())
        c.mPosition = c.mStroke->total();
    c.mStroke->draw(*this, refresh);
    if (c.mPosition == c.mStroke->total())
        c.mPosition = c.mStroke->total() / 2;
}

