#pragma once

#include <string>
#include <string_view>

namespace Matcha
{
class GLTexture
{
public:
    GLTexture(uint32_t width, uint32_t height);
    GLTexture(std::string_view path);
    ~GLTexture();

    void Bind(uint32_t slot = 0) const;

    void SetData(void* data, uint32_t size);

    [[nodiscard]] uint32_t GetWidth() const;
    [[nodiscard]] uint32_t GetHeight() const;
    [[nodiscard]] const std::string& GetPath() const;

private:
    void LoadTextureFromFile(std::string_view path);

private:
    uint32_t mHandle;

    std::string mPath;

    uint32_t mWidth, mHeight;
    GLenum mInternalFormat, mDataFormat;
};
}  // namespace Matcha