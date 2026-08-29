#pragma once

#include "Graphics/Texture.h"

#include <glad/glad.h>

#include <string>
#include <string_view>

namespace Matcha
{
class GLTexture final : public Texture
{
public:
    GLTexture(uint32_t width, uint32_t height);
    GLTexture(std::string_view path);
    ~GLTexture() override;

    void Bind(uint32_t slot = 0) const override;

    void SetData(void* data, uint32_t size) override;

    [[nodiscard]] uint32_t GetWidth() const override;
    [[nodiscard]] uint32_t GetHeight() const override;
    [[nodiscard]] const std::string& GetPath() const override;

private:
    void LoadTextureFromFile(std::string_view path);

private:
    uint32_t m_Handle;

    std::string m_Path;

    uint32_t m_Width, m_Height;
    GLenum m_InternalFormat, m_DataFormat;
};
}  // namespace Matcha
