#pragma once

#include "IndexBuffer.h"
#include "Math/Vector.h"
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

    // Local-space (pre-worldMatrix) bounds, computed once by ResourceManager::CreateMesh from the
    // position attribute of the vertices it was built from - used for viewport ray-picking, which
    // has no other way to reach vertex data once it's uploaded GPU-side.
    Vector3 localBoundsMin;
    Vector3 localBoundsMax;
};
}  // namespace Matcha
