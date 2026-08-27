#include "FrameBuffer.h"
#include "Core/Assert.h"
#include "GL/GLFrameBuffer.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
{
    switch (GetRendererAPI())
    {
    case RendererAPI::API::OpenGL:
        return std::make_unique<GLFrameBuffer>(spec);
    case RendererAPI::API::None:
    case RendererAPI::API::Vulkan:
    case RendererAPI::API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}
}  // namespace Matcha
