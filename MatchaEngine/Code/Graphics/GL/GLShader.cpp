#include "GLShader.h"
#include "Core/Logger.h"
#include "GLShaderUtils.h"

#include <fstream>
#include <filesystem>
#include <vector>
#include <format>

namespace Matcha
{
GLShader::GLShader(std::string_view name, std::span<const std::string> paths)
    : m_Handle(glCreateProgram())
{
    for (const auto& p : paths)
    {
        if (auto result = ParseFile(p); !result)
        {
            MT_CORE_ERROR("Failed to create shader: {0}", result.error());

            return;
        }
    }

    // Every source file parsed successfully - same "only report a path once it's confirmed
    // loaded" reasoning as GLTexture::LoadTextureFromFile. Separate from CreateProgram() below
    // (GL link step) succeeding or not: these are still genuinely this shader's source files
    // even if linking then fails.
    SetPaths(std::vector<std::string>(paths.begin(), paths.end()));

    if (auto result = CreateProgram(m_Sources, m_Handle); !result)
        MT_CORE_ERROR("Failed to create shader: {0}", result.error());
}

GLShader::~GLShader()
{
    glDeleteProgram(m_Handle);
}

void GLShader::Bind() const
{
    glUseProgram(m_Handle);
}

void GLShader::Unbind() const
{
    glUseProgram(0);
}

bool GLShader::Reload()
{
    std::vector<std::string> paths;
    paths.reserve(m_Sources.size());

    for (const auto& [type, source] : m_Sources)
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

    auto result = CreateProgram(m_Sources, newHandle);

    if (!result)
    {
        MT_CORE_ERROR("Failed to reload shader: {0}", result.error());
        glDeleteProgram(newHandle);
        return false;
    }

    if (m_Handle != 0)
        glDeleteProgram(m_Handle);

    m_Handle = newHandle;

    std::string joinedPaths;

    for (size_t i = 0; i < paths.size(); ++i)
    {
        if (i > 0)
            joinedPaths += ", ";

        joinedPaths += paths[i];
    }

    MT_CORE_INFO("Reloaded shader: {0}", joinedPaths);

    return true;
}

GLint GLShader::GetUniformLocation(const std::string& name)
{
    // Check if it's already cached
    if (m_LocationCache.find(name) != m_LocationCache.end())
        return m_LocationCache[name];

    // Otherwise, query the driver once and cache it
    GLint location = glGetUniformLocation(m_Handle, name.c_str());
    m_LocationCache[name] = location;
    return location;
}

void GLShader::SetMat4(std::string_view name, const Matrix4& value)
{
    GLint location = GetUniformLocation(std::string(name));
    glProgramUniformMatrix4fv(m_Handle, location, 1, GL_FALSE, value.GetData());
}

void GLShader::SetFloat(std::string_view name, float value)
{
    GLint location = GetUniformLocation(std::string(name));
    glProgramUniform1f(m_Handle, location, value);
}

void GLShader::SetFloat3(std::string_view name, const Vector3& value)
{
    GLint location = GetUniformLocation(std::string(name));
    glProgramUniform3f(m_Handle, location, value.x, value.y, value.z);
}

void GLShader::SetFloat4(std::string_view name, const Vector4& value)
{
    GLint location = GetUniformLocation(std::string(name));
    glProgramUniform4f(m_Handle, location, value.x, value.y, value.z, value.w);
}

void GLShader::SetInt(std::string_view name, int value)
{
    GLint location = GetUniformLocation(std::string(name));
    glProgramUniform1i(m_Handle, location, value);
}

GLuint GLShader::GetHandle() const
{
    return m_Handle;
}

std::expected<void, std::string> GLShader::ParseFile(const std::string& path)
{
    std::filesystem::path filePath(path);

    if (!filePath.has_extension())
        return std::unexpected(std::format("Shader file does not have an extension: {}", path));

    GLenum shaderType = ShaderTypeFromString(filePath.extension().string());

    if (shaderType == GL_NONE)
        return std::unexpected(std::format("Shader type does not exist: {}", filePath.extension().string()));

    std::ifstream file(path, std::ios::in | std::ios::binary);

    if (!file.is_open())
        return std::unexpected(std::format("Failed to open file: {}", path));

    std::string source{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    m_Sources[shaderType] = std::make_pair(path, source);

    return {};
}
}  // namespace Matcha