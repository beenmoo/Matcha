#pragma once

#include "Graphics/Renderer.h"
#include "Graphics/ResourceManager.h"

namespace Matcha
{
class Application;
class Input;
class Logger;
class Time;
class Window;

class Context
{
public:
    Context(Application& application,
            Input& input,
            Time& time,
            Window& window,
            Renderer& renderer,
            ResourceManager& resourceManager);

    [[nodiscard]] Application& GetApplication();
    [[nodiscard]] const Application& GetApplication() const;
    [[nodiscard]] Input& GetInput();
    [[nodiscard]] const Input& GetInput() const;
    [[nodiscard]] Time& GetTime();
    [[nodiscard]] const Time& GetTime() const;
    [[nodiscard]] Window& GetWindow();
    [[nodiscard]] const Window& GetWindow() const;
    [[nodiscard]] Renderer& GetRenderer();
    [[nodiscard]] const Renderer& GetRenderer() const;
    [[nodiscard]] ResourceManager& GetResourceManager();
    [[nodiscard]] const ResourceManager& GetResourceManager() const;

private:
    Application& m_Application;
    Input& m_Input;
    Time& m_Time;
    Window& m_Window;
    Renderer& m_Renderer;
    ResourceManager& m_ResourceManager;
};
}  // namespace Matcha