#pragma once

#include <cstdint>
#include <memory>

namespace Matcha
{
class UniformBuffer
{
public:
    virtual ~UniformBuffer() = default;

    virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

};
}  // namespace Matcha
