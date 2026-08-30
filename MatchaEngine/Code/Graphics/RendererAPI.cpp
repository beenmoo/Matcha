#include "RendererAPI.h"
#include "Core/Assert.h"
#include "GL/GLRendererAPI.h"

namespace Matcha
{
namespace
{
RendererAPI::API s_API = RendererAPI::API::OpenGL;
}

RendererAPI::API GetRendererAPI()
{
    return s_API;
}

void SetRendererAPI(RendererAPI::API api)
{
    s_API = api;
}

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

std::string RendererAPI::ToString(RendererAPI::API api)
{
    switch (s_API)
    {
    case RendererAPI::API::OpenGL:
        return "OpenGL";
    case RendererAPI::API::Vulkan:
        return "Vulkan";
    case RendererAPI::API::DirectX12:
        return "DirectX12";
    default:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return "Unknown";
    }
}
}  // namespace Matcha
