#include "RendererAPI.h"
#include "Core/Assert.h"
#include "GL/GLRendererAPI.h"

namespace Matcha
{
std::unique_ptr<RendererAPI> RendererAPI::Create(API api)
{
    switch (api)
    {
    case API::OpenGL:
        return std::make_unique<GLRendererAPI>();
    case API::None:
    case API::Vulkan:
    case API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}

std::string RendererAPI::ToString(API api)
{
    switch (api)
    {
    case API::None:
        return "None";
    case API::OpenGL:
        return "OpenGL";
    case API::Vulkan:
        return "Vulkan";
    case API::DirectX12:
        return "DirectX12";
    }

    MT_ASSERT(false, "Unknown RendererAPI!");
    return "Unknown";
}
}  // namespace Matcha
