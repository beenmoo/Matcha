#include "GLRenderer.h"
#include "Core/Assert.h"
#include "Core/Core.h"

#include <algorithm>
#include <glad/glad.h>

namespace Matcha
{
GLRenderer::GLRenderer(GLResourceManager& resourceManager) : mResourceManager(resourceManager)
{
}

void GLRenderer::Submit(const RenderData& renderData)
{
    mRenderData.emplace_back(renderData);
}

void GLRenderer::Flush()
{
    SortRenderData();

    for (const auto& renderData : mRenderData)
    {
        auto* mesh = mResourceManager.GetMesh(renderData.mesh);
        auto* shader = mResourceManager.GetShader(renderData.shader);

        MT_ASSERT(mesh, "Submitted RenderData references an unknown mesh handle!");
        MT_ASSERT(shader, "Submitted RenderData references an unknown shader handle!");

        mesh->vertexArray.Bind();
        shader->Bind();

        if (renderData.texture.IsValid())
        {
            auto* texture = mResourceManager.GetTexture(renderData.texture);

            MT_ASSERT(texture, "Submitted RenderData references an unknown texture handle!");

            texture->Bind(0);
        }

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(renderData.indexCount), GL_UNSIGNED_INT, nullptr);
    }

    mRenderData.clear();
}

void GLRenderer::SortRenderData()
{
    std::sort(mRenderData.begin(), mRenderData.end(), [](const RenderData& a, const RenderData& b) {
        if (a.shader.GetID() != b.shader.GetID())
            return a.shader.GetID() < b.shader.GetID();

        return a.texture.GetID() < b.texture.GetID();
    });
}
}  // namespace Matcha
