#include "VertexArray.h"
#include "Core/Assert.h"
#include "GL/GLVertexArray.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<VertexArray> VertexArray::Create()
{
    switch (GetRendererAPI())
    {
    case RendererAPI::API::OpenGL:
        return std::make_unique<GLVertexArray>();
    case RendererAPI::API::None:
    case RendererAPI::API::Vulkan:
    case RendererAPI::API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}
}  // namespace Matcha
