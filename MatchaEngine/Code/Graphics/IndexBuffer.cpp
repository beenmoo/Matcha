#include "IndexBuffer.h"
#include "RendererAPI.h"

namespace Matcha
{
std::shared_ptr<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count)
{
    return GetActiveRendererAPI().CreateIndexBuffer(indices, count);
}
}  // namespace Matcha
