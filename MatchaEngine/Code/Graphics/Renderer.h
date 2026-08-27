#pragma once

#include "RenderHandles.h"
#include "RendererAPI.h"
#include "ResourceManager.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"

#include <cstdint>
#include <vector>

namespace Matcha
{
struct RenderData
{
    MeshHandle mesh;
    ShaderHandle shader;
    TextureHandle texture;
    uint32_t indexCount = 0;
    Matrix4 transform;
};

class Renderer
{
public:
    Renderer(RendererAPI& rendererAPI, ResourceManager& resourceManager);

    void Submit(const RenderData& renderData);
    void Flush();
    void Clear();
    void SetClearColor(const Vector4& color);

private:
    void SortRenderData();

private:
    RendererAPI& m_RendererAPI;
    ResourceManager& m_ResourceManager;

    std::vector<RenderData> m_RenderData;
};
}  // namespace Matcha
