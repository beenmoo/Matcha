#pragma once

#include "Graphics/UniformBuffer.h"

namespace Matcha
{
class GLUniformBuffer final : public UniformBuffer
{
public:
    GLUniformBuffer(uint32_t size, uint32_t binding);
    ~GLUniformBuffer() override;

    void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

private:
    uint32_t m_Handle = 0;
};
}  // namespace Matcha
