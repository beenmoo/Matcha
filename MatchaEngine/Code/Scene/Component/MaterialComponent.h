#pragma once

#include "Graphics/RenderHandles.h"
#include "Math/Vector.h"

namespace Matcha
{
struct MaterialComponent
{
    ShaderHandle shader;

    // Default-constructed (invalid) means no texture bound.
    TextureHandle texture;

    Vector4 albedoColor = Vector4(1.0f);
};
}  // namespace Matcha
