#include "GLRenderer.h"
#include "Core/Core.h"

#include <algorithm>

namespace Matcha
{
GLRenderer::GLRenderer()
{
}

void GLRenderer::Submit(const GLRenderData& renderData)
{
    mRenderData.emplace_back(renderData);
}

void GLRenderer::Flush()
{
    // Sort the render data based on shader ID and texture ID to minimize state changes
    SortRenderData();

    // Draw
    for (const auto& renderData : mRenderData)
    {
        mVertexArray.Bind();
        glUseProgram(renderData.shaderHandle);
        glBindTextureUnit(0, renderData.textureHandle);
        glDrawElements(GL_TRIANGLES, renderData.indexCount, GL_UNSIGNED_INT, nullptr);
    }

    mRenderData.clear();
}

void GLRenderer::SortRenderData()
{
    std::sort(mRenderData.begin(), mRenderData.end(), [](const GLRenderData& a, const GLRenderData& b) {
        if (a.shaderHandle != b.shaderHandle)
        {
            return a.shaderHandle < b.shaderHandle;
        }

        return a.textureHandle < b.textureHandle;
    });
}
}  // namespace Matcha