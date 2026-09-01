#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace Matcha
{
class Texture
{
public:
    virtual ~Texture() = default;

    virtual void Bind(uint32_t slot = 0) const = 0;

    virtual void SetData(void* data, uint32_t size) = 0;

    [[nodiscard]] virtual uint32_t GetWidth() const = 0;
    [[nodiscard]] virtual uint32_t GetHeight() const = 0;
    [[nodiscard]] const std::string& GetPath() const
    {
        return m_Path;
    }

    [[nodiscard]] static std::unique_ptr<Texture> Create(uint32_t width, uint32_t height);
    [[nodiscard]] static std::unique_ptr<Texture> Create(std::string_view path);

protected:
    void SetPath(std::string_view path)
    {
        m_Path = path;
    }

private:
    std::string m_Path;
};
}  // namespace Matcha
