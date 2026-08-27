#pragma once

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

    [[nodiscard]] virtual uint32_t GetHandle() const = 0;

    [[nodiscard]] static std::unique_ptr<Shader> Create(std::string_view name, const std::initializer_list<std::string>& paths);
};
}  // namespace Matcha
