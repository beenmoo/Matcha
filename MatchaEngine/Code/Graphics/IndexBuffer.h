#pragma once

#include <vector>
#include <glad/glad.h>

namespace Matcha
{
    class IndexBuffer
    {
    public:
        IndexBuffer(uint32_t* indices, uint32_t count);
        ~IndexBuffer();

        void Bind() const;
        void Unbind() const;

        void AddIndices(std::initializer_list<uint32_t> indices);
        void SetIndices(uint32_t* indices, uint32_t count);
        void SetIndicesNew(uint32_t* indices, uint32_t count);
        void Clear();

        uint32_t GetCount() const;

    private:
        uint32_t mObjectID;

        GLenum mDrawType = GL_STATIC_DRAW;

        std::vector<uint32_t> mIndices;
    };
}