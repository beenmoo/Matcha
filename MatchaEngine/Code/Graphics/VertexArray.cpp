#include "VertexArray.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<VertexArray> VertexArray::Create()
{
    return GetActiveRendererAPI().CreateVertexArray();
}
}  // namespace Matcha
