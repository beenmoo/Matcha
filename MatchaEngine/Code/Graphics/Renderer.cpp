#include "Renderer.h"
#include "Core/Assert.h"

#include <algorithm>

namespace Matcha
{
namespace
{
// LightBlock's std140 layout (see StandardMesh.frag):
//   offset  0: int u_NumLights           (padded out to 16 bytes - the vec3 below needs 16-align)
//   offset 16: vec3 padding
//   offset 32: vec3 u_AmbientColor, float u_AmbientStrength packed into its trailing 4 bytes
//   offset 48: Light u_Lights[MAX_LIGHTS], 64 bytes each (see LightData in Renderer.h)
// Unlike LightData, this can't be a plain mirrored C++ struct: std140 forces a 12-byte gap
// between the int and the vec3 that follows it (16-byte alignment) that C++'s own alignof(vec3)
// (4, not 16) would never insert on its own - so the header is written via hand-placed offsets.
constexpr uint32_t LightArrayOffset = 48;
}  // namespace


Renderer::Renderer(RendererAPI& rendererAPI, ResourceManager& resourceManager)
    : m_RendererAPI(rendererAPI),
      m_ResourceManager(resourceManager)
{
}

void Renderer::Init()
{
    // std140 pads a vec3 following a mat4 out to a full vec4 (16 bytes), even though only 12 are
    // used - the buffer has to be sized for that padded layout, not the tight C++ one.
    m_CameraUniformBuffer = UniformBuffer::Create(sizeof(Matrix4) + sizeof(Vector4), 0);

    m_LightUniformBuffer = UniformBuffer::Create(LightArrayOffset + MAX_LIGHTS * sizeof(LightData), 1);

    SetLights({});
    SetAmbient(0.1f, Vector3(1.0f));

    m_DefaultWhiteTexture = m_ResourceManager.CreateTexture(1, 1);

    uint8_t whitePixel[4] = {255, 255, 255, 255};
    m_ResourceManager.GetTexture(m_DefaultWhiteTexture)->SetData(whitePixel, sizeof(whitePixel));
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
        shader->SetInt("u_AlbedoMap", 0);
        shader->SetFloat("u_SpecularStrength", renderData.specularStrength);
        shader->SetFloat("u_Shininess", renderData.shininess);

        TextureHandle textureHandle = renderData.texture.IsValid() ? renderData.texture : m_DefaultWhiteTexture;
        auto* texture = m_ResourceManager.GetTexture(textureHandle);

        MT_ASSERT(texture, "Submitted RenderData references an unknown texture handle!");

        texture->Bind(0);

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

void Renderer::SetCameraPosition(const Vector3& position)
{
    Vector4 padded(position.x, position.y, position.z, 0.0f);
    m_CameraUniformBuffer->SetData(&padded, sizeof(Vector4), sizeof(Matrix4));
}

void Renderer::SetLights(const std::vector<LightData>& lights)
{
    int32_t count = static_cast<int32_t>(std::min(lights.size(), static_cast<size_t>(MAX_LIGHTS)));

    m_LightUniformBuffer->SetData(&count, sizeof(count), 0);

    if (count > 0)
        m_LightUniformBuffer->SetData(lights.data(), sizeof(LightData) * static_cast<size_t>(count), LightArrayOffset);
}

void Renderer::SetAmbient(float strength, const Vector3& color)
{
    // u_AmbientColor (offset 32, 12 bytes) is immediately followed by u_AmbientStrength (offset
    // 44, packed into the vec3's std140 trailing pad) - contiguous, so written in one call.
    float data[4] = {color.x, color.y, color.z, strength};

    m_LightUniformBuffer->SetData(data, sizeof(data), 32);
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
