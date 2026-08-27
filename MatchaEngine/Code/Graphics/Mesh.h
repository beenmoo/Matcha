#pragma once

#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <memory>

namespace Matcha
{
struct Mesh
{
    std::unique_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    std::shared_ptr<IndexBuffer> indexBuffer;
};
}  // namespace Matcha
