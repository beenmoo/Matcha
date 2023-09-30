#pragma once

#include <vector>
#include <memory>

namespace Matcha
{
    class VertexBuffer;
    class IndexBuffer;

    class VertexArray
    {
    public:
        VertexArray();
        ~VertexArray();

        void Bind() const;
        void Unbind() const;

        void InitAttributes(uint32_t vbIndex);
        void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer);
        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer);

    private:
        uint32_t mObjectID;

        std::vector<std::shared_ptr<VertexBuffer>> mVertexBuffers;
        std::shared_ptr<IndexBuffer> mIndexBuffer = nullptr;
    };
}