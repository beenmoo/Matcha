#include "Texture.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<Texture> Texture::Create(uint32_t width, uint32_t height)
{
    return GetActiveRendererAPI().CreateTexture(width, height);
}

std::unique_ptr<Texture> Texture::Create(std::string_view path)
{
    return GetActiveRendererAPI().CreateTexture(path);
}
}  // namespace Matcha
