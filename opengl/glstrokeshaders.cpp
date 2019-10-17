#include "glstrokeshaders.h"

const char * GLStrokeShaders::STROKE_VERTEX_SHADER =
        "#version 410\n"
        "#extension GL_ARB_explicit_uniform_location : enable\n"

        "layout(location=0) in vec2 position;\n"
        "layout(location=1) in float width;\n"
        "layout(location= 2) uniform float scale = 1.0;\n"
        "out float gs_width;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position * scale, 0.0f, 1.0f);\n"
        "    gs_width = width * scale;\n"
        "}\n"
        ;

const char * GLStrokeShaders::STROKE_GEOMETRY_SHADER =
        "#version 410\n"
        "#extension GL_ARB_explicit_uniform_location : enable\n"

        "layout (lines_adjacency) in;\n"
        "layout (triangle_strip, max_vertices = 14) out;\n"

        "layout(location= 0) uniform vec4 color;\n"
        "layout(location=1) uniform vec2 pixelsize;\n"

        "in float gs_width[];\n"
        "out vec4 gs_color;\n"

        "vec4 toBezier(float t, vec4 p0, vec4 p1, vec4 p2) // 二阶\n"
        "{\n"
        "    float t2 = t * t;\n"
        "    float one_minus_t = 1.0 - t;\n"
        "    float one_minus_t2 = one_minus_t * one_minus_t;\n"
        "    return (p0 * one_minus_t2 + p1 * 2.0 * t * one_minus_t + p2 * t2);\n"
        "}\n"

        "vec4 tangent(float t, vec4 p0, vec4 p1, vec4 p2) // 二阶切线\n"
        "{\n"
        "    return (p0 + p2 - p1 * 2.0) * t + (p1 - p0);\n"
        "}\n"

        "void drawPoint(vec4 p, vec4 dir, float width)\n"
        "{\n"
        "    vec4 v = normalize(vec4(-dir.y / pixelsize[1], dir.x / pixelsize[0], dir.zw));\n"
        "    vec4 v2 = vec4(v.x * pixelsize[0], v.y * pixelsize[1], v.zw) * width / 2;\n"
        "    gs_color = p;\n"
        "    gl_Position = p + v2;\n"
        "    gs_color = color;\n"
        "    EmitVertex();\n"
        "    gl_Position = p - v2;\n"
        "    gs_color = color;\n"
        "    EmitVertex();\n"
        "}\n"

        "void main(void)\n"
        "{\n"
        "    //float w = 0.01f;\n"
        "    vec4 p0 = (gl_in[0].gl_Position + gl_in[1].gl_Position) / 2;\n"
        "    vec4 p1 = gl_in[1].gl_Position;\n"
        "    vec4 p2 = (gl_in[1].gl_Position + gl_in[2].gl_Position) / 2;\n"
        "    vec4 p3 = gl_in[2].gl_Position;\n"
        "    float w0 = (gs_width[0] + gs_width[1]) / 2;\n"
        "    float w1 = gs_width[1];\n"
        "    float w2 = (gs_width[1] + gs_width[2]) / 2;\n"
        "    float w3 = gs_width[2];\n"
        "    if (gl_in[0].gl_Position[0] < -1.0) {\n"
        "        drawPoint(p1, p2 - p1, w1);\n"
        "        drawPoint(p2, p2 - p1, w2);\n"
        "        EndPrimitive();\n"
        "        return;\n"
        "    }\n"
        "    drawPoint(p0, p1 - p0, w0);\n"
        "    for (float t = 0.2f; t < 1.0; t += 0.2f)\n"
        "    {\n"
        "        vec4 p = toBezier(t, p0, p1, p2);\n"
        "        vec4 d = tangent(t, p0, p1, p2);\n"
        "        drawPoint(p, d, w1);\n"
        "    }\n"
        "    drawPoint(p2, p3 - p2, w2);\n"
        "    if (gl_in[3].gl_Position[0] < -1.0)\n"
        "    {\n"
        "        drawPoint(p3, p3 - p2, w3);\n"
        "    }\n"
        "    EndPrimitive();\n"
        "}\n"
        ;


const char * GLStrokeShaders::STROKE_GEOMETRY_SHADER2 =
        "#version 410\n"
        "#extension GL_ARB_explicit_uniform_location : enable\n"

        "layout (lines_adjacency) in;\n"
        "layout (triangle_strip, max_vertices = 14) out;\n"

        "layout(location= 0) uniform vec4 color;\n"
        "layout(location=1) uniform vec2 pixelsize;\n"

        "in float gs_width[];\n"
        "out vec4 gs_color;\n"

        "void drawPoint(vec4 p, vec4 dir, float width)\n"
        "{\n"
        "    vec4 v = normalize(vec4(-dir.y / pixelsize[1], dir.x / pixelsize[0], dir.zw));\n"
        "    vec4 v2 = vec4(v.x * pixelsize[0], v.y * pixelsize[1], v.zw) * width / 2;\n"
        "    gs_color = color;\n"
        "    gl_Position = p + v2;\n"
        "    EmitVertex();\n"
        "    gl_Position = p - v2;\n"
        "    EmitVertex();\n"
        "}\n"

        "void main(void)\n"
        "{\n"
        "    vec4 p1 = gl_in[1].gl_Position;\n"
        "    vec4 p2 = gl_in[2].gl_Position;\n"
        "    float w1 = gs_width[1];\n"
        "    float w2 = gs_width[2];\n"
        "    drawPoint(p1, p2 - p1, w1);\n"
        "    drawPoint(p2, p2 - p1, w2);\n"
        "    EndPrimitive();\n"
        "}\n"
        ;

const char * GLStrokeShaders::STROKE_FRAGMENT_SHADER =
        "#version 410\n"
        "#extension GL_ARB_explicit_uniform_location : enable\n"
        "layout(location=0) uniform vec4 color;\n"
        "layout(location=0) out vec4 fragColor;\n"
        "in vec4 gs_color;\n"
        "void main()\n"
        "{\n"
        "     fragColor = gs_color;\n"
        "}\n"
        ;

/* background */

const char * GLStrokeShaders::BACKGROUND_VERTEX_SHADER =
        "#version 330\n"
        "layout(location=0) in vec2 position;\n"
        "layout(location=1) in vec2 inputTextureCoordinate;\n"
        "out vec2 textureCoordinate;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0.0f, 1.0f);\n"
        "    textureCoordinate = inputTextureCoordinate.xy;\n"
        "}\n"
        ;

const char * GLStrokeShaders::BACKGROUND_FRAGMENT_SHADER =
        "#version 330\n"
        "#extension GL_ARB_explicit_uniform_location : enable\n"
        "in vec2 textureCoordinate;\n"
        "layout(location=0) uniform sampler2D inputImageTexture;\n"
        "layout(location=0) out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "     fragColor = texture(inputImageTexture, textureCoordinate);\n"
        "}\n"
        ;

const float GLStrokeShaders::sVertexData[] = {
        -1.0, -1.0, //0.0,
        1.0, -1.0, //0.0,
        -1.0, 1.0, //0.0,
        1.0, 1.0, //0.0,
        0.0,1.0,
        1.0,1.0,
        0.0,0.0,
        1.0,0.0,
};

GLStrokeShaders::GLStrokeShaders()
{
}

