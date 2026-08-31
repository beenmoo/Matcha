#pragma once

#include "IndexBuffer.h"
#include "VertexBuffer.h"

#include <memory>

namespace Matcha
{
class VertexArray
{
public:
    virtual ~VertexArray() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer> buffer) = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer> buffer) = 0;

    [[nodiscard]] static std::unique_ptr<VertexArray> Create();
};
}  // namespace Matcha
