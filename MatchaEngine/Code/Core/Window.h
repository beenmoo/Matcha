#pragma once

#include "Math/Vector.h"
#include "Core/Event.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace Matcha
{
class Input;

struct WindowSpecification
{
    std::string m_Title = "Application";
    int m_Width = 1280;
    int m_Height = 720;
    std::optional<Vector2Int> m_Position;
    bool m_Resizable = true;

    // SDL-only: creates the native window hidden and never presents to it (SwapBuffers() becomes
    // a no-op) - used when a host (MatchaEditor) renders into its own FrameBuffer and displays that
    // texture itself, rather than SDL ever putting anything on screen directly. See SDLWindow.
    bool m_Headless = false;
};

enum class WindowBackend
{
    SDL
};

class Window
{
public:
    using WindowSpecification = Matcha::WindowSpecification;

public:
    explicit Window(const WindowSpecification& spec);
    virtual ~Window() = default;

    virtual void Resize(int width, int height) = 0;
    virtual void SwapBuffers() = 0;
    virtual void ProcessEvents(const Event& evt) = 0;

    // Registers the callback every event gets delivered to, translated to Matcha's Event type.
    // Persistent (like SetContextReadyCallback below), not per-call: a host embedding this window
    // (MatchaEditor's EngineViewportWidget, via SDLWindow::DispatchExternalEvent) can deliver
    // events asynchronously, any time, not just while PumpEvents() runs.
    virtual void SetEventDispatch(std::function<void(const Event&)> dispatch) = 0;

    // Delivers every event that arrived since the last call to the registered dispatch callback -
    // pumps SDL_PollEvent and translates each one.
    virtual void PumpEvents() = 0;

    // Invoked once the window's GL context is actually current and ready to load function
    // pointers against. Fires synchronously, before the constructor returns - SDL's context is
    // current immediately.
    virtual void SetContextReadyCallback(std::function<void()> callback) = 0;

    // Makes this window's GL context current on the calling thread. Application::Tick() already
    // calls this at the top of every frame - only needed elsewhere for GL calls issued outside the
    // normal frame loop (e.g. one-off resource creation triggered from editor UI code, or
    // MatchaEditor's EngineViewportWidget reclaiming the engine's context after its own separate
    // Qt-owned context was made current to blit the previous frame's result).
    virtual void MakeContextCurrent() = 0;

    // Every backend stores its live size/title/etc in m_WindowSpec below and calls
    // HandleResizeEvent() from its own ProcessEvents() override, so these never need overriding.
    [[nodiscard]] int GetWidth() const
    {
        return m_WindowSpec.m_Width;
    }

    [[nodiscard]] int GetHeight() const
    {
        return m_WindowSpec.m_Height;
    }

    [[nodiscard]] Vector2Int GetCenter() const
    {
        return Vector2Int(m_WindowSpec.m_Width / 2, m_WindowSpec.m_Height / 2);
    }

    [[nodiscard]] float GetAspectRatio() const
    {
        return static_cast<float>(m_WindowSpec.m_Width) / m_WindowSpec.m_Height;
    }

    [[nodiscard]] const WindowSpecification& GetWindowSpecification() const
    {
        return m_WindowSpec;
    }

    [[nodiscard]] virtual bool IsMinimized() const = 0;

    // input is wired to SDLInput so SetCursorLockState can reach SDL_SetWindowRelativeMouseMode on
    // the native window it creates (see SDLWindow's constructor / SDLInput::SetNativeWindow).
    [[nodiscard]] static std::unique_ptr<Window> Create(WindowBackend backend, const WindowSpecification& spec = WindowSpecification(), Input* input = nullptr);

protected:
    // Applies a WindowResized event's new size to m_WindowSpec and updates the GL viewport -
    // shared by every backend's ProcessEvents() override.
    void HandleResizeEvent(const Event& evt);

protected:
    WindowSpecification m_WindowSpec;
};
}  // namespace Matcha
