#include "Shader.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<Shader> Shader::Create(std::string_view name, std::span<const std::string> paths)
{
    return GetActiveRendererAPI().CreateShader(name, paths);
}
}  // namespace Matcha
