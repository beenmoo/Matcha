#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <initializer_list>
#include <utility>
#include <expected>
#include <glad/glad.h>

namespace Matcha
{
class GLShader
{
public:
    GLShader(
        std::string_view name,
        const std::initializer_list<std::string>& paths);
    ~GLShader();

    void Bind() const;
    void Unbind() const;

    bool Reload();

    [[nodiscard]] GLuint GetHandle() const;

private:
    [[nodiscard("error must be handled")]] std::expected<void, std::string> ParseFile(const std::string& path);

private:
    GLuint m_Handle;
    std::unordered_map<GLenum, std::pair<std::string, std::string>> m_Sources;
};
}  // namespace Matcha