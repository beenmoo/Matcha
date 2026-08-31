#include "UniformBuffer.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
{
    return GetActiveRendererAPI().CreateUniformBuffer(size, binding);
}
}  // namespace Matcha
