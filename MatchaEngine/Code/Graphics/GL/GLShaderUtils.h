#pragma once

#include "Graphics/ShaderDataType.h"

#include <glad/glad.h>
#include <cstdint>
#include <expected>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Matcha
{
[[nodiscard]] std::expected<void, std::string> CreateProgram(const std::unordered_map<GLenum, std::pair<std::string, std::string>>& sources, GLuint handle);

[[nodiscard]] GLenum ShaderTypeFromString(const std::string& type);

[[nodiscard]] std::vector<char> GetShaderErrorInfo(int32_t id, GLenum statusType);

[[nodiscard]] uint32_t ShaderDataTypeSize(ShaderDataType type);

[[nodiscard]] GLenum ShaderDataTypeToGLDataType(ShaderDataType type);
}  // namespace Matcha
