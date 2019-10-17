#include "glstroke.h"
#include "glstrokerenderer.h"

GLStroke::GLStroke(ushort capcity, ushort capcity2)
    : mPointBuffer(QOpenGLBuffer::VertexBuffer)
    , mIndiceBuffer(QOpenGLBuffer::IndexBuffer)
{
    mPoints.resize(capcity * POINT_SIZE);
    mIndices.resize(capcity * 7 / 5 + 4);
    mPointBufferCapacity = static_cast<ushort>(capcity2 + 2);
    mIndiceBufferCapacity = static_cast<ushort>(capcity2 * 7 / 5 + 4);
    mPointBufferCount = 2;
}

void GLStroke::push(float pt[])
{
    if (mGroupCount == 0)
    {
        if (mIndiceCount >= mIndices.size())
            mIndices.resize(mIndices.size() * 2);
        if (total() > 0)
            mIndices[mIndiceCount++] = 0xffff; // Start of group
        mIndices[mIndiceCount++] = 0; // Start of group
    }
    if (mIndiceCount >= mIndices.size())
        mIndices.resize(mIndices.size() * 2);
    mIndices[mIndiceCount++] = mPointBufferCount + mPointCount;
    if (mPointCount * POINT_SIZE >= mPoints.size())
        mPoints.resize(mPoints.size() * 2);
    mPoints[mPointCount * POINT_SIZE] = pt[0];
    mPoints[mPointCount * POINT_SIZE + 1] = pt[1];
    mPoints[mPointCount * POINT_SIZE + 2] = pt[2];
    ++mPointCount;
    ++mGroupCount;
}

void GLStroke::push()
{
    if (mIndiceCount >= mIndices.size())
        mIndices.resize(mIndices.size() * 2);
    if (mGroupCount > 0)
    {
        mIndices[mIndiceCount++] = 1; // End of group
        mGroupCount = 0;
    }
}

ushort GLStroke::sync(OpenGL & gl)
{
    if (mIndiceCount == 0)
        return mIndiceBufferCount;
    ushort indice = mIndiceCount;
    if (mGroupCount != 0)
    {
        if (mIndiceCount >= mIndices.size())
            mIndices.resize(mIndices.size() * 2);
        mIndices[indice++] = 0; // Temperary end of group
    }
    ushort start = mIndiceBufferCount;
    if (mIndices[0] == 0xffff)
        ++start;
    else if (mIndices[1] == 0xffff)
        start += 2;
    else if (start >= 2)
        start -= 2;
    // sync indices
    if (mIndiceBufferCount + indice > mIndiceBufferCapacity)
        resize(gl, mPointBuffer);
    mIndiceBuffer.bind();
    mIndiceBuffer.write(INDICE_STRIDE * mIndiceBufferCount, &mIndices[0],
            INDICE_STRIDE * indice);
    gl.assertGL();
    mIndiceBufferCount = mIndiceBufferCount + mIndiceCount;
    // sync vertexs
    if (mPointBufferCount + mPointCount > mPointBufferCapacity)
        resize(gl, mPointBuffer);
    mPointBuffer.bind();
    mPointBuffer.write(POINT_STRIDE * mPointBufferCount, &mPoints[0],
            POINT_STRIDE * mPointCount);
    gl.assertGL();
    mPointBufferCount = mPointBufferCount + mPointCount;
    mPointCount = 0;
    mIndiceCount = 0;
    return start;
}

void GLStroke::release(OpenGL & gl)
{
    releaseBuffer(gl);
}

void GLStroke::clear()
{
    mPointCount = 0;
    mIndiceCount = 0;
    mGroupCount = 0;
    mPointBufferCount = 2;
    mIndiceBufferCount = 0;
}

void GLStroke::draw(OpenGL & gl, bool refresh)
{
    if (!mPointArray.isCreated())
        allocBuffer(gl);
    ushort start = sync(gl);
    if (refresh)
        start = 0;
    if (total() <= start)
        return;
    mPointArray.bind();
    qDebug() << "draw" << start << " ->" << total();
    gl.glDrawElements(GL_LINE_STRIP_ADJACENCY, total() - start, GL_UNSIGNED_SHORT,
                   reinterpret_cast<GLvoid*>(static_cast<intptr_t>(start * INDICE_STRIDE)));
    mPointArray.release();
}

void GLStroke::allocBuffer(OpenGL & gl)
{
    mPointArray.create();
    mPointBuffer.create();
    mPointBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mIndiceBuffer.create();
    mIndiceBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    mPointArray.bind();
    mIndiceBuffer.bind();
    mIndiceBuffer.allocate(nullptr, INDICE_STRIDE * mIndiceBufferCapacity);
    gl.assertGL();
    mPointBuffer.bind();
    mPointBuffer.allocate(nullptr, POINT_STRIDE * mPointBufferCapacity);
    gl.assertGL();
    mPointBuffer.write(0, &mPoints[0], POINT_STRIDE * mPointBufferCount);
    gl.assertGL();
    gl.glVertexAttribPointer(GLStrokeRenderer::VERTEX_POINT, POINT_SIZE - 1, GL_FLOAT, false, POINT_STRIDE, nullptr);
    gl.assertGL();
    gl.glEnableVertexAttribArray(GLStrokeRenderer::VERTEX_POINT);
    gl.assertGL();
    gl.glVertexAttribPointer(GLStrokeRenderer::VERTEX_WIDTH, 1, GL_FLOAT, false, POINT_STRIDE,
                             reinterpret_cast<GLvoid *>(sizeof(float) * (POINT_SIZE - 1)));
    gl.assertGL();
    gl.glEnableVertexAttribArray(GLStrokeRenderer::VERTEX_WIDTH);
    gl.assertGL();
    mPointArray.release();
    gl.assertGL();
}

void GLStroke::resize(OpenGL & gl, QOpenGLBuffer & buffer)
{
    (void)gl;
    if (buffer.isCreated()) {
        throw std::exception();
    }
}

void GLStroke::releaseBuffer(OpenGL & gl)
{
    (void)gl;
    mPointArray.destroy();
    mPointBuffer.destroy();
    mIndiceBuffer.destroy();
}
