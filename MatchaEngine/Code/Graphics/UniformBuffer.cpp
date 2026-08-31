#include "UniformBuffer.h"
#include "Core/Assert.h"
#include "GL/GLUniformBuffer.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
{
    switch (GetRendererAPI())
    {
    case RendererAPI::API::OpenGL:
        return std::make_unique<GLUniformBuffer>(size, binding);
    case RendererAPI::API::None:
    case RendererAPI::API::Vulkan:
    case RendererAPI::API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}
}  // namespace Matcha
