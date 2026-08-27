#pragma once

#include "Graphics/RenderHandles.h"

namespace Matcha
{
struct MaterialComponent
{
    ShaderHandle shader;

    // Default-constructed (invalid) means no texture bound.
    TextureHandle texture;
};
}  // namespace Matcha
