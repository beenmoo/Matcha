#include "SDLWindow.h"
#include "Core/Assert.h"
#include "SDLInput.h"

#include <glad/glad.h>
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

SDLWindow::SDLWindow(const WindowSpecification& spec, Input* input)
    : Window(spec)
{
    InitContext();

    if (auto* sdlInput = dynamic_cast<SDLInput*>(input))
        sdlInput->SetNativeWindow(m_NativeWindow);
}

SDLWindow::~SDLWindow()
{
    if (m_GLContext)
        SDL_GL_DestroyContext(m_GLContext);

    if (m_NativeWindow)
        SDL_DestroyWindow(m_NativeWindow);

    // Paired with the SDL_Init() in InitContext() - owned here (not Application) since this is
    // the only place that ever calls SDL_Init() in the first place.
    SDL_Quit();
}

void SDLWindow::ProcessEvents(const Event& evt)
{
    HandleResizeEvent(evt);
}

void SDLWindow::SetEventDispatch(std::function<void(const Event&)> dispatch)
{
    m_EventDispatch = std::move(dispatch);
}

void SDLWindow::PumpEvents()
{
    SDL_Event sdlEvent;

    while (SDL_PollEvent(&sdlEvent))
    {
        std::optional<Event> evt = TranslateEvent(sdlEvent);

        if (evt && m_EventDispatch)
            m_EventDispatch(*evt);
    }
}

void SDLWindow::SetContextReadyCallback(std::function<void()> callback)
{
    // SDL's GL context is already current by the time the constructor returns, so there's
    // nothing to defer - fire immediately.
    if (callback)
        callback();
}

void SDLWindow::SetTickCallback(std::function<void()> callback)
{
    // Unused: Run()'s own blocking loop calls Application::Tick() directly for SDL.
}

void SDLWindow::Resize(int width, int height)
{
    SDL_SetWindowSize(m_NativeWindow, width, height);
}

void SDLWindow::SwapBuffers()
{
    SDL_GL_SwapWindow(m_NativeWindow);
}

bool SDLWindow::IsMinimized() const
{
    return SDL_GetWindowFlags(m_NativeWindow) & SDL_WINDOW_MINIMIZED;
}

void SDLWindow::InitContext()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        MT_CORE_ERROR("{}", SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    uint32_t flags = SDL_WINDOW_OPENGL;

    if (m_WindowSpec.m_Resizable)
        flags |= SDL_WINDOW_RESIZABLE;

    m_NativeWindow = SDL_CreateWindow(GetWindowSpecification().m_Title.c_str(),
                                       GetWindowSpecification().m_Width,
                                       GetWindowSpecification().m_Height,
                                       flags);

    MT_ASSERT(m_NativeWindow, SDL_GetError());

    if (m_WindowSpec.m_Position)
        SDL_SetWindowPosition(m_NativeWindow, m_WindowSpec.m_Position->x, m_WindowSpec.m_Position->y);
    else
        SDL_SetWindowPosition(m_NativeWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    m_GLContext = SDL_GL_CreateContext(m_NativeWindow);
    SDL_GL_MakeCurrent(m_NativeWindow, m_GLContext);

    int status = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    MT_ASSERT(status, SDL_GetError());
}
}  // namespace Matcha
