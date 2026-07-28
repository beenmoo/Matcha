#pragma once

#include "Math/Matrix.h"

#include <glad/glad.h>

namespace Matcha
{
    struct GLRenderData
    {
        GLuint vao;
        GLuint vbo;
        GLuint ibo;
        GLsizei indexCount;
        Matrix4 transform;

        GLRenderData() = default;
        GLRenderData(
            GLuint vao, 
            GLuint vbo, 
            GLuint ibo, 
            GLsizei indexCount, 
            const Matrix4& transform) : 
            vao(vao), 
            vbo(vbo), 
            ibo(ibo), 
            indexCount(indexCount), 
            transform(transform) 
        {}
    };
}