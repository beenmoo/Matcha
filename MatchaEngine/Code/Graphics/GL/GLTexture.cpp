#include "GLTexture.h"
#include "Core/Assert.h"

#include <stb_image.h>

namespace Matcha
{
GLTexture::GLTexture(uint32_t width, uint32_t height) : m_Width(width),
                                                        m_Height(height)
{
    m_InternalFormat = GL_RGBA8;
    m_DataFormat = GL_RGBA;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_Handle);
    glTextureStorage2D(m_Handle, 1, m_InternalFormat, m_Width, m_Height);

    glTextureParameteri(m_Handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_Handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(m_Handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_Handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

GLTexture::GLTexture(std::string_view path)
{
    LoadTextureFromFile(path);
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &m_Handle);
}

void GLTexture::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_Handle);
}

void GLTexture::SetData(void* data, uint32_t size)
{
    uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;

    MT_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");

    glTextureSubImage2D(m_Handle, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}

uint32_t GLTexture::GetWidth() const
{
    return m_Width;
}

uint32_t GLTexture::GetHeight() const
{
    return m_Height;
}

const std::string& GLTexture::GetPath() const
{
    return m_Path;
}

void GLTexture::LoadTextureFromFile(std::string_view path)
{
    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    stbi_uc* data = stbi_load(path.data(), &width, &height, &channels, 0);

    if (data)
    {
        m_Width = width;
        m_Height = height;

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

        m_InternalFormat = internalFormat;
        m_DataFormat = dataFormat;

        MT_ASSERT(internalFormat & dataFormat, "Format not supported!");

        glCreateTextures(GL_TEXTURE_2D, 1, &m_Handle);
        glTextureStorage2D(m_Handle, 1, internalFormat, m_Width, m_Height);

        glTextureParameteri(m_Handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_Handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_Handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_Handle, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(m_Handle, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
}
}  // namespace Matcha