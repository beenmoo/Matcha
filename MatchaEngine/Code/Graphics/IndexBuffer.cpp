#include "IndexBuffer.h"
#include "Core/Assert.h"
#include "GL/GLIndexBuffer.h"
#include "RendererAPI.h"

namespace Matcha
{
std::shared_ptr<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count)
{
    switch (GetRendererAPI())
    {
    case RendererAPI::API::OpenGL:
        return std::make_shared<GLIndexBuffer>(indices, count);
    case RendererAPI::API::None:
    case RendererAPI::API::Vulkan:
    case RendererAPI::API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}
}  // namespace Matcha
