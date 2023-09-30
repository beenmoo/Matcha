#pragma once

#include <string>
#include <string_view>

namespace Matcha
{
    class Texture
    {
    public:
        Texture(uint32_t width, uint32_t height);
        Texture(std::string_view path);
        ~Texture();

        void Bind(uint32_t slot = 0) const;

        void SetData(void* data, uint32_t size);

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        const std::string& GetPath() const;

    private:
        void LoadTextureFromFile(std::string_view path);

    private:
        uint32_t mObjectID;

        std::string mPath;

        uint32_t mWidth, mHeight;
        GLenum mInternalFormat, mDataFormat;
    };
}