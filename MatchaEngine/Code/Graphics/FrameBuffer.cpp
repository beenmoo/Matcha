#include "FrameBuffer.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
{
    return GetActiveRendererAPI().CreateFrameBuffer(spec);
}
}  // namespace Matcha
