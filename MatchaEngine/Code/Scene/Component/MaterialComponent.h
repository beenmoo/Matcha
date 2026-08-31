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

    // Blinn-Phong specular. specularStrength scales the highlight's intensity; shininess controls
    // its tightness - higher values (smoother/glossier surfaces) produce a smaller, sharper
    // highlight, lower values (rougher surfaces) spread it out.
    float specularStrength = 0.5f;
    float shininess = 32.0f;
};
}  // namespace Matcha
