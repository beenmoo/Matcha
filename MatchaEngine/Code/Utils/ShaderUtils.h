#pragma once

#include "Core/Core.h"

#include <string>
#include <glad/glad.h>
#include <unordered_map>
#include <vector>

namespace Matcha
{
    struct ShaderUtils
    {
        enum class ShaderDataType
        {
            None = 0, 
            Float, 
            Float2, 
            Float3, 
            Float4, 
            Mat3, 
            Mat4, 
            Int, 
            Int2, 
            Int3, 
            Int4, 
            Bool
        };

        static GLenum ShaderTypeFromString(const std::string& type)
        {
            if (type == "vert")
                return GL_VERTEX_SHADER;
            if (type == "frag")
                return GL_FRAGMENT_SHADER;

            return GL_NONE;
        }

#ifdef MT_DEBUG
        static std::vector<char> GetShaderErrorInfo(int32_t id, GLenum statusType)
        {
            std::vector<char> infoLog;
            int32_t maxLength;

            switch (statusType)
            {
            case GL_COMPILE_STATUS:
                glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
                infoLog.resize(maxLength);

                glGetShaderInfoLog(id, maxLength, nullptr, infoLog.data());
                break;
            case GL_LINK_STATUS:
                glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);
                infoLog.resize(maxLength);

                glGetProgramInfoLog(id, maxLength, nullptr, infoLog.data());
                break;
            default:
                break;
            }

            return infoLog;
        }
#endif

        static uint32_t ShaderDataTypeSize(ShaderDataType type)
        {
            switch (type)
            {
            case ShaderDataType::Float:    
                return 4;
            case ShaderDataType::Float2:   
                return 4 * 2;
            case ShaderDataType::Float3:   
                return 4 * 3;
            case ShaderDataType::Float4:   
                return 4 * 4;
            case ShaderDataType::Mat3:     
                return 4 * 3 * 3;
            case ShaderDataType::Mat4:     
                return 4 * 4 * 4;
            case ShaderDataType::Int:      
                return 4;
            case ShaderDataType::Int2:     
                return 4 * 2;
            case ShaderDataType::Int3:     
                return 4 * 3;
            case ShaderDataType::Int4:     
                return 4 * 4;
            case ShaderDataType::Bool:     
                return 1;
            default:
                break;
            }

            return 0;
        }

        static GLenum ShaderDataTypeToGLDataType(ShaderDataType type)
        {
            switch (type)
            {
            case ShaderDataType::Float:
                return GL_FLOAT;
            case ShaderDataType::Float2:
                return GL_FLOAT;
            case ShaderDataType::Float3:
                return GL_FLOAT;
            case ShaderDataType::Float4:
                return GL_FLOAT;
            case ShaderDataType::Mat3:
                return GL_FLOAT;
            case ShaderDataType::Mat4:
                return GL_FLOAT;
            case ShaderDataType::Int:
                return GL_INT;
            case ShaderDataType::Int2:
                return GL_INT;
            case ShaderDataType::Int3:
                return GL_INT;
            case ShaderDataType::Int4:
                return GL_INT;
            case ShaderDataType::Bool:
                return GL_BOOL;
            default:
                break;
            }

            return 0;
        }
    };
}