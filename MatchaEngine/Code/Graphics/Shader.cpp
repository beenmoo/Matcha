#include "Shader.h"
#include "Core/Assert.h"
#include "GL/GLShader.h"
#include "RendererAPI.h"

namespace Matcha
{
std::unique_ptr<Shader> Shader::Create(std::string_view name, const std::initializer_list<std::string>& paths)
{
    switch (GetRendererAPI())
    {
    case RendererAPI::API::OpenGL:
        return std::make_unique<GLShader>(name, paths);
    case RendererAPI::API::None:
    case RendererAPI::API::Vulkan:
    case RendererAPI::API::DirectX12:
        MT_ASSERT(false, "RendererAPI not yet supported!");
        return nullptr;
    }

    return nullptr;
}
}  // namespace Matcha
