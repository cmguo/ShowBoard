#ifndef GLSTROKESHADERS_H
#define GLSTROKESHADERS_H

#include "opengl.h"

class GLStrokeShaders : protected OpenGL
{
public:
    static constexpr int VERTEX_POINT = 0;
    static constexpr int VERTEX_WIDTH = 1;
    static constexpr int VERTEX_TEXTURE = 1;
    static constexpr int UNIFORM_COLOR = 0;
    static constexpr int UNIFORM_TEXTURE = 0;
    static constexpr int UNIFORM_PIXEL_SIZE = 1;
    static constexpr int UNIFORM_SCALE = 2;

    /* stroke */

    static const char * STROKE_VERTEX_SHADER;

    static const char * STROKE_GEOMETRY_SHADER;

    static const char * STROKE_GEOMETRY_SHADER2;

    static const char * STROKE_FRAGMENT_SHADER;

    /* background */

    static const char * BACKGROUND_VERTEX_SHADER;

    static const char * BACKGROUND_FRAGMENT_SHADER;

    static const float sVertexData[16];

protected:
    GLStrokeShaders();
};

#endif // GLSTROKESHADERS_H
