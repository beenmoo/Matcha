#include "GLShaderUtils.h"
#include "Graphics/ShaderDataType.h"

#include <format>

namespace Matcha::Utils
{
std::vector<char> GetShaderErrorInfo(int32_t id, GLenum statusType)
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

std::expected<void, std::string> CreateProgram(const std::unordered_map<GLenum, std::pair<std::string, std::string>>& sources, GLuint handle)
{
    if (sources.find(GL_VERTEX_SHADER) == sources.end() ||
        sources.find(GL_FRAGMENT_SHADER) == sources.end())
        return std::unexpected("Shader requires both a vertex and fragment source");

    GLint success = 0;
    std::expected<void, std::string> result;

    std::vector<GLuint> shaderIDs;

    for (const auto& s : sources)
    {
        GLuint shaderID = glCreateShader(s.first);
        shaderIDs.emplace_back(shaderID);

        const char* code = s.second.second.c_str();

        glShaderSource(shaderID, 1, &code, nullptr);
        glCompileShader(shaderID);

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            std::vector<GLchar> infoLog = GetShaderErrorInfo(shaderID, GL_COMPILE_STATUS);

            result = std::unexpected(std::format("Shader compilation failed ({}):\n{}", s.second.first, infoLog.data()));

            break;
        }

        glAttachShader(handle, shaderID);
    }

    if (success)
    {
        glLinkProgram(handle);
        glGetProgramiv(handle, GL_LINK_STATUS, &success);

        if (!success)
        {
            std::vector<GLchar> infoLog = GetShaderErrorInfo(handle, GL_LINK_STATUS);

            result = std::unexpected(std::format("Shader linking failed:\n{}", infoLog.data()));
        }
    }

    for (const auto& id : shaderIDs)
        glDeleteShader(id);

    return result;
}

GLenum ShaderTypeFromString(const std::string& type)
{
    if (type == ".vert")
        return GL_VERTEX_SHADER;
    if (type == ".frag")
        return GL_FRAGMENT_SHADER;

    return GL_NONE;
}

uint32_t ShaderDataTypeSize(ShaderDataType type)
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

GLenum ShaderDataTypeToGLDataType(ShaderDataType type)
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
}  // namespace Matcha::Utils
