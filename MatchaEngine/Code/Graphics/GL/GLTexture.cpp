#include "GLTexture.h"
#include "Core/Assert.h"

#include <stb_image.h>

namespace Matcha
{
GLTexture::GLTexture(uint32_t width, uint32_t height) : mWidth(width),
                                                        mHeight(height)
{
    mInternalFormat = GL_RGBA8;
    mDataFormat = GL_RGBA;

    glCreateTextures(GL_TEXTURE_2D, 1, &mHandle);
    glTextureStorage2D(mHandle, 1, mInternalFormat, mWidth, mHeight);

    glTextureParameteri(mHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(mHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(mHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

GLTexture::GLTexture(std::string_view path)
{
    LoadTextureFromFile(path);
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &mHandle);
}

void GLTexture::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, mHandle);
}

void GLTexture::SetData(void* data, uint32_t size)
{
    uint32_t bpp = mDataFormat == GL_RGBA ? 4 : 3;

    MT_ASSERT(size == mWidth * mHeight * bpp, "Data must be entire texture!");

    glTextureSubImage2D(mHandle, 0, 0, 0, mWidth, mHeight, mDataFormat, GL_UNSIGNED_BYTE, data);
}

uint32_t GLTexture::GetWidth() const
{
    return mWidth;
}

uint32_t GLTexture::GetHeight() const
{
    return mHeight;
}

const std::string& GLTexture::GetPath() const
{
    return mPath;
}

void GLTexture::LoadTextureFromFile(std::string_view path)
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

        glCreateTextures(GL_TEXTURE_2D, 1, &mHandle);
        glTextureStorage2D(mHandle, 1, internalFormat, mWidth, mHeight);

        glTextureParameteri(mHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(mHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(mHandle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(mHandle, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(mHandle, 0, 0, 0, mWidth, mHeight, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
}
}  // namespace Matcha