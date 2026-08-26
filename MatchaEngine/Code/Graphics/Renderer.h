#pragma once

#include "RenderHandles.h"
#include "Math/Matrix.h"

#include <cstdint>

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
    virtual ~Renderer() = default;

    virtual void Submit(const RenderData& renderData) = 0;
    virtual void Flush() = 0;
};
}  // namespace Matcha
