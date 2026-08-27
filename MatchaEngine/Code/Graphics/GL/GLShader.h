#pragma once

#include "Graphics/Shader.h"

#include <unordered_map>
#include <string>
#include <string_view>
#include <initializer_list>
#include <utility>
#include <expected>
#include <glad/glad.h>

namespace Matcha
{
class GLShader final : public Shader
{
public:
    GLShader(
        std::string_view name,
        const std::initializer_list<std::string>& paths);
    ~GLShader() override;

    void Bind() const override;
    void Unbind() const override;

    bool Reload() override;

    [[nodiscard]] uint32_t GetHandle() const override;

private:
    [[nodiscard("error must be handled")]] std::expected<void, std::string> ParseFile(const std::string& path);

private:
    GLuint m_Handle;
    std::unordered_map<GLenum, std::pair<std::string, std::string>> m_Sources;
};
}  // namespace Matcha
