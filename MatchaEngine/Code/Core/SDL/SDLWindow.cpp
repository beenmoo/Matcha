#include "SDLWindow.h"
#include "Core/Assert.h"
#include "SDLInput.h"

#include <glad/glad.h>
#include <optional>

namespace Matcha
{
namespace
{
// SDL's own mouse-button constants (SDL_BUTTON_LEFT etc.) aren't a contiguous 0-based enum like
// Matcha::MouseButton, so this needs an explicit table rather than a cast - mirrors KeyCode below,
// which SDL_Scancode's numbering happens to match directly.
std::optional<MouseButton> ToMatchaMouseButton(Uint8 sdlButton)
{
    switch (sdlButton)
    {
    case SDL_BUTTON_LEFT:
        return MouseButton::Left;
    case SDL_BUTTON_MIDDLE:
        return MouseButton::Middle;
    case SDL_BUTTON_RIGHT:
        return MouseButton::Right;
    case SDL_BUTTON_X1:
        return MouseButton::Back;
    case SDL_BUTTON_X2:
        return MouseButton::Forward;
    default:
        return std::nullopt;
    }
}

std::optional<Event> TranslateEvent(const SDL_Event& sdlEvent)
{
    switch (sdlEvent.type)
    {
    case SDL_EVENT_QUIT:
        return Event{.type = EventType::Quit};
    case SDL_EVENT_WINDOW_RESIZED:
        return Event{.type = EventType::WindowResized, .width = sdlEvent.window.data1, .height = sdlEvent.window.data2};
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        return Event{.type = EventType::WindowFocusLost};
    case SDL_EVENT_MOUSE_MOTION:
        return Event{.type = EventType::MouseMoved, .x = sdlEvent.motion.xrel, .y = sdlEvent.motion.yrel};
    case SDL_EVENT_MOUSE_WHEEL:
        return Event{.type = EventType::MouseScrolled, .x = sdlEvent.wheel.x, .y = sdlEvent.wheel.y};
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        return Event{.type = EventType::JoystickMoved, .x = static_cast<float>(sdlEvent.jaxis.value), .axis = sdlEvent.jaxis.axis};
    // KeyCode's values are literal SDL scancodes (see KeyCodes.h), so this is a direct cast, not a
    // lookup table - same convention QtKeyCodeMap documents for the Qt side of this translation.
    case SDL_EVENT_KEY_DOWN:
        if (!sdlEvent.key.repeat)
            return Event{.type = EventType::KeyDown, .key = static_cast<KeyCode>(sdlEvent.key.scancode)};
        return std::nullopt;
    case SDL_EVENT_KEY_UP:
        return Event{.type = EventType::KeyUp, .key = static_cast<KeyCode>(sdlEvent.key.scancode)};
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (std::optional<MouseButton> button = ToMatchaMouseButton(sdlEvent.button.button))
            return Event{.type = EventType::MouseButtonDown, .mouseButton = *button};
        return std::nullopt;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (std::optional<MouseButton> button = ToMatchaMouseButton(sdlEvent.button.button))
            return Event{.type = EventType::MouseButtonUp, .mouseButton = *button};
        return std::nullopt;
    default:
        return std::nullopt;
    }
}
}  // namespace

SDLWindow::SDLWindow(const WindowSpecification& spec, Input* input)
    : Window(spec)
{
    InitContext();

    m_Input = dynamic_cast<SDLInput*>(input);

    if (m_Input)
        m_Input->SetNativeWindow(m_NativeWindow);
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

    // Settles this frame's key/mouse-button state - see SDLInput::ApplyPendingInput()'s own
    // comment for why this can't just write straight into current state as events arrive. Must run
    // after the loop above (which, via m_EventDispatch -> Application's registered lambda ->
    // Input::ProcessEvents(), is what populates the pending buffer for real SDL events) and after
    // any external host has pushed its own events for this frame via DispatchExternalEvent, but
    // before m_ExternalPumpCallback below, which may read the now-settled state (e.g. RMB-held for
    // cursor-lock polling).
    if (m_Input)
        m_Input->ApplyPendingInput();

    if (m_ExternalPumpCallback)
        m_ExternalPumpCallback();
}

void SDLWindow::SetContextReadyCallback(std::function<void()> callback)
{
    // SDL's GL context is already current by the time the constructor returns, so there's
    // nothing to defer - fire immediately.
    if (callback)
        callback();
}

void SDLWindow::MakeContextCurrent()
{
    // Real, not a no-op: under MatchaEditor, EngineViewportWidget makes its own separate Qt-owned
    // context current on this same thread every frame (to blit the previous frame's result) - this
    // has to reclaim the engine's own context before any engine GL call, not just assume it's still
    // current.
    //
    // The clear-then-set dance is load-bearing, not defensive: SDL caches which context it believes
    // is current and turns SDL_GL_MakeCurrent() into a no-op when the arguments match that cache.
    // Qt makes its own context current through its own platform code, which SDL never sees - so
    // SDL's cache goes stale, and a plain call here "succeeds" without actually switching anything,
    // leaving Qt's context current for the whole engine frame. Vertex array objects are not shared
    // across a share group, so the very next draw binds a VAO that doesn't exist in that context,
    // which the driver answers with an access violation rather than a GL error. Releasing the
    // context first never matches the cache, forcing a real switch.
    SDL_GL_MakeCurrent(m_NativeWindow, nullptr);

    if (!SDL_GL_MakeCurrent(m_NativeWindow, m_GLContext))
        MT_CORE_ERROR("SDL_GL_MakeCurrent failed: {}", SDL_GetError());
}

void SDLWindow::Resize(int width, int height)
{
    SDL_SetWindowSize(m_NativeWindow, width, height);
}

void SDLWindow::SwapBuffers()
{
    // Headless (MatchaEditor): nothing to present - this window is never shown, the engine renders
    // into its own FrameBuffer instead (see Editor), which a Qt widget displays separately.
    if (!m_WindowSpec.m_Headless)
        SDL_GL_SwapWindow(m_NativeWindow);
}

void SDLWindow::DispatchExternalEvent(const Event& evt)
{
    if (m_EventDispatch)
        m_EventDispatch(evt);
}

void SDLWindow::SetExternalPumpCallback(std::function<void()> callback)
{
    m_ExternalPumpCallback = std::move(callback);
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

    // MatchaEditor: this window only ever exists to own a real GL context/FrameBuffer target -
    // never shown, never focused, never receives real input (Qt's own widget owns all of that -
    // see DispatchExternalEvent).
    if (m_WindowSpec.m_Headless)
        flags |= SDL_WINDOW_HIDDEN;

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
