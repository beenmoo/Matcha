#pragma once

#include "Math/Vector.h"
#include "Utils/ShaderUtils.h"

#include <vector>
#include <initializer_list>

namespace Matcha
{
    class VertexBuffer
    {
    public:
        class BufferLayout
        {
        private:
            struct BufferElement
            {
                ShaderUtils::ShaderDataType mType = ShaderUtils::ShaderDataType::None;
                uint32_t mSize = 0;
                size_t mOffset = 0;
                bool mNormalized = false;

                BufferElement(ShaderUtils::ShaderDataType type, bool normalized = false) :
                    mType(type), 
                    mSize(ShaderUtils::ShaderDataTypeSize(type)),
                    mNormalized(normalized)
                {}

                uint32_t GetComponentCount() const
                {
                    switch (mType)
                    {
                    case ShaderUtils::ShaderDataType::Float:   
                        return 1;
                    case ShaderUtils::ShaderDataType::Float2:  
                        return 2;
                    case ShaderUtils::ShaderDataType::Float3:  
                        return 3;
                    case ShaderUtils::ShaderDataType::Float4:  
                        return 4;
                    case ShaderUtils::ShaderDataType::Mat3:    
                        return 3; 
                    case ShaderUtils::ShaderDataType::Mat4:    
                        return 4; 
                    case ShaderUtils::ShaderDataType::Int:     
                        return 1;
                    case ShaderUtils::ShaderDataType::Int2:    
                        return 2;
                    case ShaderUtils::ShaderDataType::Int3:    
                        return 3;
                    case ShaderUtils::ShaderDataType::Int4:    
                        return 4;
                    case ShaderUtils::ShaderDataType::Bool:    
                        return 1;
                    default:
                        break;
                    }

                    return 0;
                }
            };

        public:
            BufferLayout(std::initializer_list<ShaderUtils::ShaderDataType> dataTypes, bool normalized = false)
            {
                for (const auto& i : dataTypes)
                    mElements.emplace_back(BufferElement(i, normalized));

                CalculateOffsetsAndStride();
            }

            const std::vector<BufferElement>& GetElements() const
            {
                return mElements;
            }

            uint32_t GetStride() const
            {
                return mStride;
            }

        private:
            void CalculateOffsetsAndStride()
            {
                size_t offset = 0;
                mStride = 0;

                for (auto& e : mElements)
                {
                    e.mOffset += offset;
                    offset += e.mOffset;
                    mStride += e.mSize;
                }
            }

        private:
            std::vector<BufferElement> mElements;

            uint32_t mStride = 0;
        };

    public:
        VertexBuffer(uint32_t sizeInBytes = 0);
        VertexBuffer(const float* vertices, uint32_t sizeInBytes);
        ~VertexBuffer();

        void Bind() const;
        void Unbind() const;

        void AddVertex(std::initializer_list<float> vertex);
        void SetVertices(const float* vertices, uint32_t sizeInBytes);
        void SetVerticesNew(const float* vertices, uint32_t sizeInBytes);
        void SetDrawType(GLenum drawType);
        void Clear();

        void SetLayout(const std::shared_ptr<BufferLayout>& layout);
        const BufferLayout* GetLayout() const;

        uint32_t GetSizeInBytes() const;

    private:
        uint32_t mObjectID;

        GLenum mDrawType = GL_STATIC_DRAW;

        std::shared_ptr<BufferLayout> mLayout = nullptr;
        std::vector<float> mVertices;
    };
}