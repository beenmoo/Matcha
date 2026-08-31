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
}  // namespace Matcha
