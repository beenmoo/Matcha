#include "GLRenderer.h"
#include "Core/Assert.h"
#include "Core/Core.h"

#include <algorithm>
#include <glad/glad.h>

namespace Matcha
{
GLRenderer::GLRenderer(GLResourceManager& resourceManager)
    : m_ResourceManager(resourceManager)
{
}

void GLRenderer::Submit(const RenderData& renderData)
{
    m_RenderData.emplace_back(renderData);
}

void GLRenderer::Flush()
{
    SortRenderData();

    for (const auto& renderData : m_RenderData)
    {
        auto* mesh = m_ResourceManager.GetMesh(renderData.mesh);
        auto* shader = m_ResourceManager.GetShader(renderData.shader);

        MT_ASSERT(mesh, "Submitted RenderData references an unknown mesh handle!");
        MT_ASSERT(shader, "Submitted RenderData references an unknown shader handle!");

        mesh->vertexArray.Bind();
        shader->Bind();

        if (renderData.texture.IsValid())
        {
            auto* texture = m_ResourceManager.GetTexture(renderData.texture);

            MT_ASSERT(texture, "Submitted RenderData references an unknown texture handle!");

            texture->Bind(0);
        }

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(renderData.indexCount), GL_UNSIGNED_INT, nullptr);
    }

    m_RenderData.clear();
}

void GLRenderer::Clear()
{
    glClearColor(m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, m_ClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRenderer::SetClearColor(const Vector4& color)
{
    m_ClearColor = color;
}

void GLRenderer::SortRenderData()
{
    std::sort(m_RenderData.begin(), m_RenderData.end(), [](const RenderData& a, const RenderData& b) {
        if (a.shader.GetID() != b.shader.GetID())
            return a.shader.GetID() < b.shader.GetID();

        return a.texture.GetID() < b.texture.GetID();
    });
}
}  // namespace Matcha
