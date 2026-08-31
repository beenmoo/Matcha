#pragma once

#include "Graphics/Renderer.h"
#include "Graphics/RendererAPI.h"
#include "Graphics/ResourceManager.h"
#include "Scene/Scene.h"

#include <type_traits>

namespace Matcha
{
class Application;
class Input;
class Logger;
class Time;
class Window;

class EngineContext
{
public:
    explicit EngineContext(Application& application,
                           Input& input,
                           Time& time,
                           Window& window,
                           RendererAPI& rendererAPI,
                           Renderer& renderer,
                           ResourceManager& resourceManager,
                           Scene& scene);

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Application&, Application&> GetApplication(this Self& self)
    {
        return self.m_Application;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Input&, Input&> GetInput(this Self& self)
    {
        return self.m_Input;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Time&, Time&> GetTime(this Self& self)
    {
        return self.m_Time;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Window&, Window&> GetWindow(this Self& self)
    {
        return self.m_Window;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const RendererAPI&, RendererAPI&> GetRendererAPI(this Self& self)
    {
        return self.m_RendererAPI;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Renderer&, Renderer&> GetRenderer(this Self& self)
    {
        return self.m_Renderer;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const ResourceManager&, ResourceManager&> GetResourceManager(this Self& self)
    {
        return self.m_ResourceManager;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Scene&, Scene&> GetScene(this Self& self)
    {
        return self.m_Scene;
    }

private:
    Application& m_Application;
    Input& m_Input;
    Time& m_Time;
    Window& m_Window;
    RendererAPI& m_RendererAPI;
    Renderer& m_Renderer;
    ResourceManager& m_ResourceManager;
    Scene& m_Scene;
};
}  // namespace Matcha
