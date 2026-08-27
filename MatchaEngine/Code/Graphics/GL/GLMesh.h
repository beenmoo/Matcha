#pragma once

#include "GLIndexBuffer.h"
#include "GLVertexArray.h"
#include "GLVertexBuffer.h"

#include <memory>

namespace Matcha
{
struct GLMesh
{
    GLVertexArray vertexArray;
    std::shared_ptr<GLVertexBuffer> vertexBuffer;
    std::shared_ptr<GLIndexBuffer> indexBuffer;
};
}  // namespace Matcha
