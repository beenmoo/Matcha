#pragma once

#include "Graphics/Renderer.h"
#include "Graphics/ResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"

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
                           Renderer& renderer,
                           ResourceManager& resourceManager,
                           SceneManager& sceneManager);

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
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Renderer&, Renderer&> GetRenderer(this Self& self)
    {
        return self.m_Renderer;
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const ResourceManager&, ResourceManager&> GetResourceManager(this Self& self)
    {
        return self.m_ResourceManager;
    }

    // Delegates to m_SceneManager rather than holding its own Scene& - the Scene it returns can
    // be swapped out from under any caller that holds onto it (SceneManager::NewScene()/
    // OpenScene()), so this must re-fetch the live one every call rather than caching it here.
    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const Scene&, Scene&> GetScene(this Self& self)
    {
        return self.m_SceneManager.GetScene();
    }

    template <typename Self>
    [[nodiscard]] std::conditional_t<std::is_const_v<Self>, const SceneManager&, SceneManager&> GetSceneManager(this Self& self)
    {
        return self.m_SceneManager;
    }

private:
    Application& m_Application;
    Input& m_Input;
    Time& m_Time;
    Window& m_Window;
    Renderer& m_Renderer;
    ResourceManager& m_ResourceManager;
    SceneManager& m_SceneManager;
};
}  // namespace Matcha
