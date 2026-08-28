#pragma once

#include "Math/Matrix.h"
#include "Math/Vector.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

namespace Matcha
{
class Shader
{
public:
    virtual ~Shader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual bool Reload() = 0;

    virtual void SetMat4(std::string_view name, const Matrix4& value) = 0;
    virtual void SetFloat4(std::string_view name, const Vector4& value) = 0;
    virtual void SetInt(std::string_view name, int value) = 0;

    [[nodiscard]] virtual uint32_t GetHandle() const = 0;

    [[nodiscard]] static std::unique_ptr<Shader> Create(std::string_view name, const std::initializer_list<std::string>& paths);
};
}  // namespace Matcha
