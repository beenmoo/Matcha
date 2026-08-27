#include "Context.h"
#include "Application.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"

namespace Matcha
{
Context::Context(Application& application,
                 Input& input,
                 Time& time,
                 Window& window,
                 Renderer& renderer,
                 ResourceManager& resourceManager)
    : m_Application(application),
      m_Input(input),
      m_Time(time),
      m_Window(window),
      m_Renderer(renderer),
      m_ResourceManager(resourceManager)
{
}

Application& Context::GetApplication()
{
    return m_Application;
}

const Application& Context::GetApplication() const
{
    return m_Application;
}

Input& Context::GetInput()
{
    return m_Input;
}

const Input& Context::GetInput() const
{
    return m_Input;
}

Time& Context::GetTime()
{
    return m_Time;
}

const Time& Context::GetTime() const
{
    return m_Time;
}

Window& Context::GetWindow()
{
    return m_Window;
}

const Window& Context::GetWindow() const
{
    return m_Window;
}

Renderer& Context::GetRenderer()
{
    return m_Renderer;
}

const Renderer& Context::GetRenderer() const
{
    return m_Renderer;
}

ResourceManager& Context::GetResourceManager()
{
    return m_ResourceManager;
}

const ResourceManager& Context::GetResourceManager() const
{
    return m_ResourceManager;
}
}  // namespace Matcha