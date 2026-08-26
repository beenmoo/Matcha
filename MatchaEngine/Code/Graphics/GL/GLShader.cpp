#include "GLShader.h"
#include "Core/Logger.h"
#include "GLShaderUtils.h"

#include <fstream>
#include <filesystem>
#include <vector>
#include <format>

namespace Matcha
{
GLShader::GLShader(
    std::string_view name,
    const std::initializer_list<std::string>& paths) : mHandle(glCreateProgram())
{
    for (const auto& p : paths)
    {
        if (auto result = ParseFile(p); !result)
        {
            MT_CORE_ERROR("Failed to create shader: {0}", result.error());

            return;
        }
    }

    if (auto result = Utils::CreateProgram(mSources, mHandle); !result)
        MT_CORE_ERROR("Failed to create shader: {0}", result.error());
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

bool GLShader::Reload()
{
    std::vector<std::string> paths;
    paths.reserve(mSources.size());

    for (const auto& [type, source] : mSources)
        paths.push_back(source.first);

    for (const auto& path : paths)
    {
        if (auto result = ParseFile(path); !result)
        {
            MT_CORE_ERROR("Failed to reload shader: {0}", result.error());
            return false;
        }
    }

    GLuint newHandle = glCreateProgram();

    auto result = Utils::CreateProgram(mSources, newHandle);

    if (!result)
    {
        MT_CORE_ERROR("Failed to reload shader: {0}", result.error());
        glDeleteProgram(newHandle);
        return false;
    }

    if (mHandle != 0)
        glDeleteProgram(mHandle);

    mHandle = newHandle;

    return true;
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

    GLenum shaderType = Utils::ShaderTypeFromString(filePath.extension().string());

    if (shaderType == GL_NONE)
        return std::unexpected(std::format("Shader type does not exist: {}", filePath.extension().string()));

    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file.is_open())
        return std::unexpected(std::format("Failed to open file: {}", path));

    std::string source{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    mSources[shaderType] = std::make_pair(path, source);

    return {};
}
}  // namespace Matcha