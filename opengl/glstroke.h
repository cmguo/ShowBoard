#ifndef GLSTROKE_H
#define GLSTROKE_H

#include "opengl.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QDebug>

class GLStroke
{
private:
    static constexpr int POINT_SIZE = 3;
    static constexpr int POINT_STRIDE = sizeof(float) * POINT_SIZE;
    static constexpr int INDICE_STRIDE = sizeof(unsigned short);

    static constexpr float FAKE_POINTS[] = {
        -2.0, 0.0, 0.0, // Start of group, or Temperary end of group
        -2.0, 1.0, 0.0, // End of group
    };

public:
    GLStroke() : GLStroke(1024, 10240) {}

    GLStroke(ushort capcity, ushort capcity2);

    ushort total() const
    {
        return static_cast<ushort>(mIndiceBufferCount + mIndiceCount + (mGroupCount > 0 ? 1 : 0));
    }

    bool finished() const
    {
        return mGroupCount == 0;
    }

public:
    void push(float pt[]);

    void push();

    void clear();

    void draw(OpenGL & gl, bool refresh);

    ushort sync(OpenGL & gl);

    void release(OpenGL & gl);

private:
    void allocBuffer(OpenGL & gl);

    void resize(OpenGL & gl, QOpenGLBuffer & buffer);

    void releaseBuffer(OpenGL & gl);

private:
    std::vector<float> mPoints;
    ushort mPointCount = 0;
    std::vector<ushort> mIndices;
    ushort mIndiceCount = 0;
    ushort mGroupCount = 0;

    QOpenGLVertexArrayObject mPointArray;
    QOpenGLBuffer mPointBuffer; // Target.ArrayBuffer
    QOpenGLBuffer mIndiceBuffer; // Target.ElementArrayBuffer
    ushort mPointBufferCapacity = 0;
    ushort mIndiceBufferCapacity = 0;
    ushort mPointBufferCount = 0;
    ushort mIndiceBufferCount = 0;
};

#endif // GLSTROKE_H
