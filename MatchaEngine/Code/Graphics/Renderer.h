#pragma once

#include "RenderHandles.h"
#include "RendererAPI.h"
#include "ResourceManager.h"
#include "UniformBuffer.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace Matcha
{
struct RenderData
{
    MeshHandle mesh;
    ShaderHandle shader;
    TextureHandle texture;

    // World-space transform. View-projection is uploaded separately, once per frame, via
    // SetViewProjection - it is not baked in here.
    Matrix4 transform;

    Vector4 albedoColor = Vector4(1.0f);
};

class Renderer
{
public:
    Renderer(RendererAPI& rendererAPI, ResourceManager& resourceManager);

    void Submit(const RenderData& renderData);
    void Flush();
    void Clear();
    void SetClearColor(const Vector4& color);

    // Uploads the given view-projection to the CameraBlock uniform buffer (binding 0), shared by
    // every shader that declares it. Intended to be called once per frame, before Flush().
    void SetViewProjection(const Matrix4& viewProjection);

private:
    void SortRenderData();

private:
    RendererAPI& m_RendererAPI;
    ResourceManager& m_ResourceManager;

    std::unique_ptr<UniformBuffer> m_CameraUniformBuffer;

    std::vector<RenderData> m_RenderData;
};
}  // namespace Matcha
