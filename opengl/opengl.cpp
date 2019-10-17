#include "opengl.h"

OpenGL::OpenGL()
{
}

void OpenGL::initGL()
{
    initializeOpenGLFunctions();
}

void OpenGL::assertGL()
{
    GLenum code = glGetError();
    if (code != 0)
        throw std::exception();
}
