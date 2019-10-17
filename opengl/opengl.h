#ifndef OPENGL_H
#define OPENGL_H

#include <QOpenGLFunctions>

class OpenGL : protected QOpenGLFunctions
{
protected:
    OpenGL();

    void initGL();

public:
    void assertGL();

    using QOpenGLFunctions::glDrawElements;
    using QOpenGLFunctions::glVertexAttribPointer;
    using QOpenGLFunctions::glEnableVertexAttribArray;
};

#endif // OPENGL_H
