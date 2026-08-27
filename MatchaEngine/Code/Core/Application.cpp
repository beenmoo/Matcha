#include "Application.h"
#include "Graphics/GL/GLRenderer.h"
#include "Graphics/GL/GLResourceManager.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <optional>

namespace Matcha
{
namespace
{
std::optional<Event> TranslateEvent(const SDL_Event& sdlEvent)
{
    switch (sdlEvent.type)
    {
    case SDL_EVENT_QUIT:
        return Event{.type = EventType::Quit};
    case SDL_EVENT_WINDOW_RESIZED:
        return Event{.type = EventType::WindowResized, .width = sdlEvent.window.data1, .height = sdlEvent.window.data2};
    case SDL_EVENT_MOUSE_MOTION:
        return Event{.type = EventType::MouseMoved, .x = sdlEvent.motion.xrel, .y = sdlEvent.motion.yrel};
    case SDL_EVENT_MOUSE_WHEEL:
        return Event{.type = EventType::MouseScrolled, .x = sdlEvent.wheel.x, .y = sdlEvent.wheel.y};
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        return Event{.type = EventType::JoystickMoved, .x = static_cast<float>(sdlEvent.jaxis.value), .axis = sdlEvent.jaxis.axis};
    default:
        return std::nullopt;
    }
}
}  // namespace

Application::Application(const ApplicationSpecification& spec)
    : m_AppSpec(spec),
      m_ResourceManager(std::make_unique<GLResourceManager>()),
      m_Renderer(std::make_unique<GLRenderer>(static_cast<GLResourceManager&>(*m_ResourceManager))),
      m_Context(*this, m_Input, m_Time, m_Window, *m_Renderer, *m_ResourceManager)
{
    LogContext();
}

Application::~Application()
{
    SDL_Quit();
}

void Application::Run()
{
    if (m_IsRunning)
        return;

    m_IsRunning = true;

    while (m_IsRunning)
    {
        PollEvents();
        Update();
        Render();
    }
}

void Application::Quit()
{
    m_IsRunning = false;
}

void Application::OnUpdate()
{
}

void Application::OnRender()
{
}

void Application::OnEvent(const Event& event)
{
}

void Application::Update()
{
    m_Input.Update();
    m_Time.Update();
    m_ResourceManager->ReloadModifiedShaders();

    OnUpdate();
}

void Application::Render()
{
    m_Renderer->Clear();
    OnRender();
    m_Renderer->Flush();
    m_Window.SwapBuffers();
}

void Application::PollEvents()
{
    SDL_Event sdlEvent;

    while (SDL_PollEvent(&sdlEvent))
    {
        std::optional<Event> evt = TranslateEvent(sdlEvent);

        if (!evt)
            continue;

        if (evt->type == EventType::Quit)
            Quit();

        m_Input.ProcessEvents(*evt);
        m_Window.ProcessEvents(*evt);

        OnEvent(*evt);
    }
}

void Application::LogContext()
{
    int matchaEngineMajor = 0, matchaEngineMinor = 1;

    MT_CORE_INFO("MatchaEngine v{}.{}", matchaEngineMajor, matchaEngineMinor);
#ifdef MT_PLATFORM_WINDOWS
    MT_CORE_INFO("Platform: WINDOWS");
#endif
#ifdef MT_PLATFORM_LINUX
    MT_CORE_INFO("Platform: LINUX");
#endif

    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    MT_CORE_INFO("OpenGL Info:");
    MT_CORE_INFO("  Vendor: {0}", vendor);
    MT_CORE_INFO("  Renderer: {0}", renderer);
    MT_CORE_INFO("  Version: {0}", version);

    MT_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 6),
              "Matcha requires at least OpenGL version 4.6!");

    int sdlVersion = SDL_GetVersion();

    MT_CORE_INFO("SDL v{}.{}.{}", SDL_VERSIONNUM_MAJOR(sdlVersion), SDL_VERSIONNUM_MINOR(sdlVersion), SDL_VERSIONNUM_MICRO(sdlVersion));
}
}  // namespace Matcha