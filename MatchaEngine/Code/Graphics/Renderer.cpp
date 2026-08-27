#include "Renderer.h"
#include "Core/Assert.h"

#include <algorithm>

namespace Matcha
{
Renderer::Renderer(RendererAPI& rendererAPI, ResourceManager& resourceManager)
    : m_RendererAPI(rendererAPI),
      m_ResourceManager(resourceManager)
{
}

void Renderer::Init()
{
    m_CameraUniformBuffer = UniformBuffer::Create(sizeof(Matrix4), 0);
}

void Renderer::Submit(const RenderData& renderData)
{
    m_RenderData.emplace_back(renderData);
}

void Renderer::Flush()
{
    SortRenderData();

    for (const auto& renderData : m_RenderData)
    {
        auto* mesh = m_ResourceManager.GetMesh(renderData.mesh);
        auto* shader = m_ResourceManager.GetShader(renderData.shader);

        MT_ASSERT(mesh, "Submitted RenderData references an unknown mesh handle!");
        MT_ASSERT(shader, "Submitted RenderData references an unknown shader handle!");

        shader->Bind();
        shader->SetMat4("u_WorldMatrix", renderData.transform);
        shader->SetFloat4("u_AlbedoColor", renderData.albedoColor);

        if (renderData.texture.IsValid())
        {
            auto* texture = m_ResourceManager.GetTexture(renderData.texture);

            MT_ASSERT(texture, "Submitted RenderData references an unknown texture handle!");

            texture->Bind(0);
        }

        m_RendererAPI.DrawIndexed(*mesh->vertexArray, mesh->indexBuffer->GetCount());
    }

    m_RenderData.clear();
}

void Renderer::Clear()
{
    m_RendererAPI.Clear();
}

void Renderer::SetClearColor(const Vector4& color)
{
    m_RendererAPI.SetClearColor(color);
}

void Renderer::SetViewProjection(const Matrix4& viewProjection)
{
    m_CameraUniformBuffer->SetData(viewProjection.GetData(), sizeof(Matrix4));
}

void Renderer::SortRenderData()
{
    std::sort(m_RenderData.begin(), m_RenderData.end(), [](const RenderData& a, const RenderData& b) {
        if (a.shader.GetID() != b.shader.GetID())
            return a.shader.GetID() < b.shader.GetID();

        return a.texture.GetID() < b.texture.GetID();
    });
}
}  // namespace Matcha
