#include "GLShader.h"
#include "Core/Logger.h"
#include "Utils/ShaderUtils.h"

#include <fstream>
#include <filesystem>
#include <vector>
#include <format>

namespace Matcha
{
GLShader::GLShader(
    std::string_view name,
    const std::initializer_list<std::string>& paths) : mHandle(glCreateProgram()),
                                                       mName(name)
{
    for (const auto& p : paths)
    {
        if (auto result = ParseFile(p); !result)
        {
            MT_CORE_ERROR("Failed to create shader: {0}\n{1}", mName, result.error());

            return;
        }
    }

    if (auto result = CreateProgram(); !result)
        MT_CORE_ERROR("Failed to create shader: {0}\n{1}", mName, result.error());
}

GLShader::~GLShader()
{
    glDeleteProgram(mHandle);
}

void GLShader::Bind() const
{
    glUseProgram(mHandle);
}

void GLShader::Unbind() const
{
    glUseProgram(0);
}

const std::string& GLShader::GetName() const
{
    return mName;
}

GLuint GLShader::GetHandle() const
{
    return mHandle;
}

std::expected<void, std::string> GLShader::ParseFile(const std::string& path)
{
    std::filesystem::path filePath(path);

    if (!filePath.has_extension())
        return std::unexpected(std::format("Shader file does not have an extension: {}", path));

    GLenum shaderType = ShaderUtils::ShaderTypeFromString(filePath.extension().string());

    if (shaderType == GL_NONE)
        return std::unexpected(std::format("Shader type does not exist: {}", filePath.extension().string()));

    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file.is_open())
        return std::unexpected(std::format("Failed to open file: {}", path));

    std::string source{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    mSources[shaderType] = std::make_pair(path, source);

    return {};
}

std::expected<void, std::string> GLShader::CreateProgram()
{
    if (mSources.find(GL_VERTEX_SHADER) == mSources.end() ||
        mSources.find(GL_FRAGMENT_SHADER) == mSources.end())
        return std::unexpected("Shader requires both a vertex and fragment source");

    GLint success = 0;
    std::expected<void, std::string> result;

    std::vector<GLuint> shaderIDs;

    for (const auto& s : mSources)
    {
        GLuint shaderID = glCreateShader(s.first);
        shaderIDs.emplace_back(shaderID);

        const char* code = s.second.second.c_str();

        glShaderSource(shaderID, 1, &code, nullptr);
        glCompileShader(shaderID);

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            std::vector<GLchar> infoLog = ShaderUtils::GetShaderErrorInfo(shaderID, GL_COMPILE_STATUS);

            result = std::unexpected(std::format("Shader compilation failed ({}):\n{}", s.second.first, infoLog.data()));

            break;
        }

        glAttachShader(mHandle, shaderID);
    }

    if (success)
    {
        glLinkProgram(mHandle);
        glGetProgramiv(mHandle, GL_LINK_STATUS, &success);

        if (!success)
        {
            std::vector<GLchar> infoLog = ShaderUtils::GetShaderErrorInfo(mHandle, GL_LINK_STATUS);

            result = std::unexpected(std::format("Shader linking failed ({}):\n{}", mName, infoLog.data()));
        }
    }

    for (const auto& id : shaderIDs)
        glDeleteShader(id);

    return result;
}
}  // namespace Matcha