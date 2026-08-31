#include "Texture.h"
#include "Core/Assert.h"
#include "GL/GLTexture.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<Texture> Texture::Create(uint32_t width, uint32_t height)
{
    switch (GetRendererAPI())
    {
    case RendererAPI::API::OpenGL:
        return std::make_unique<GLTexture>(width, height);
    case RendererAPI::API::None:
    case RendererAPI::API::Vulkan:
    case RendererAPI::API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}

std::unique_ptr<Texture> Texture::Create(std::string_view path)
{
    switch (GetRendererAPI())
    {
    case RendererAPI::API::OpenGL:
        return std::make_unique<GLTexture>(path);
    case RendererAPI::API::None:
    case RendererAPI::API::Vulkan:
    case RendererAPI::API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}
}  // namespace Matcha
