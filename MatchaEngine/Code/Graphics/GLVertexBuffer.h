#pragma once

#include "Math/Vector.h"
#include "Utils/ShaderUtils.h"

#include <vector>
#include <initializer_list>

namespace Matcha
{
    class GLVertexBuffer
    {
    public:
        class BufferLayout
        {
        private:
            struct BufferElement
            {
                ShaderUtils::ShaderDataType type = ShaderUtils::ShaderDataType::None;
                GLuint size = 0;
                size_t offset = 0;
                GLboolean normalized = false;

                BufferElement(ShaderUtils::ShaderDataType type, GLboolean normalized = false) :
                    type(type), 
                    size(ShaderUtils::ShaderDataTypeSize(type)),
                    normalized(normalized)
                {}

                uint32_t GetComponentCount() const
                {
                    switch (type)
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
            BufferLayout(std::initializer_list<ShaderUtils::ShaderDataType> dataTypes, GLboolean normalized = false)
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
                    e.offset = offset;
                    offset += e.size;
                    mStride += e.size;
                }
            }

        private:
            std::vector<BufferElement> mElements;

            GLsizei mStride = 0;
        };

    public:
        GLVertexBuffer(GLuint sizeInBytes = 0);
        GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes);
        ~GLVertexBuffer();

        void Bind() const;
        void Unbind() const;

        void AddVertex(std::initializer_list<GLfloat> vertex);
        void SetVertices(const GLfloat* vertices, GLuint sizeInBytes);
        void SetVerticesNew(const GLfloat* vertices, GLuint sizeInBytes);
        void SetDrawType(GLenum drawType);
        void Clear();

        void SetLayout(const std::shared_ptr<BufferLayout> layout);
        const BufferLayout* GetLayout() const;

        GLuint GetSizeInBytes() const;

    private:
        GLuint mObjectID;

        GLenum mDrawType = GL_STATIC_DRAW;

        std::shared_ptr<BufferLayout> mLayout = nullptr;
        std::vector<GLfloat> mVertices;
    };
}