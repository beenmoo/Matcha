#include "Shader.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<Shader> Shader::Create(std::string_view name, const std::initializer_list<std::string>& paths)
{
    return GetActiveRendererAPI().CreateShader(name, paths);
}
}  // namespace Matcha
