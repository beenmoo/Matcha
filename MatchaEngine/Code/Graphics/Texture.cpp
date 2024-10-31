#include "Texture.h"
#include "Core/Core.h"

#include <stb_image/stb_image.h>

namespace Matcha
{
    Texture::Texture(uint32_t width, uint32_t height) :
        mWidth(width),
        mHeight(height)
    {
        mInternalFormat = GL_RGBA8;
        mDataFormat = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &mObjectID);
        glTextureStorage2D(mObjectID, 1, mInternalFormat, mWidth, mHeight);

        glTextureParameteri(mObjectID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(mObjectID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(mObjectID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(mObjectID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    Texture::Texture(std::string_view path)
    {
        LoadTextureFromFile(path);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &mObjectID);
    }

    void Texture::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, mObjectID);
    }

    void Texture::SetData(void* data, uint32_t size)
    {
        uint32_t bpp = mDataFormat == GL_RGBA ? 4 : 3;

        MT_ASSERT(size == mWidth * mHeight * bpp, "Data must be entire texture!");

        glTextureSubImage2D(mObjectID, 0, 0, 0, mWidth, mHeight, mDataFormat, GL_UNSIGNED_BYTE, data);
    }

    uint32_t Texture::GetWidth() const
    {
        return mWidth;
    }

    uint32_t Texture::GetHeight() const
    {
        return mHeight;
    }

    const std::string& Texture::GetPath() const
    {
        return mPath;
    }
    
    void Texture::LoadTextureFromFile(std::string_view path)
    {
        stbi_set_flip_vertically_on_load(1);

        int width, height, channels;
        stbi_uc* data = stbi_load(path.data(), &width, &height, &channels, 0);

        if (data)
        {
            mWidth = width;
            mHeight = height;

            GLenum internalFormat = 0, dataFormat = 0;

            if (channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if (channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }

            mInternalFormat = internalFormat;
            mDataFormat = dataFormat;

            MT_ASSERT(internalFormat & dataFormat, "Format not supported!");

            glCreateTextures(GL_TEXTURE_2D, 1, &mObjectID);
            glTextureStorage2D(mObjectID, 1, internalFormat, mWidth, mHeight);

            glTextureParameteri(mObjectID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(mObjectID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTextureParameteri(mObjectID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(mObjectID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTextureSubImage2D(mObjectID, 0, 0, 0, mWidth, mHeight, dataFormat, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }
    }
}