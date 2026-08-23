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
    GLuint shaderHandle;
    GLuint textureHandle;

    GLRenderData() = default;
    GLRenderData(
        GLuint vao,
        GLuint vbo,
        GLuint ibo,
        GLsizei indexCount,
        const Matrix4& transform,
        GLuint shaderID,
        GLuint textureID) : vao(vao),
                            vbo(vbo),
                            ibo(ibo),
                            indexCount(indexCount),
                            transform(transform),
                            shaderHandle(shaderHandle),
                            textureHandle(textureHandle)
    {
    }
};
}  // namespace Matcha