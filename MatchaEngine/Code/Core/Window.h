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
};

enum class WindowBackend
{
    SDL,
    Qt
};

class Window
{
public:
    using WindowSpecification = Matcha::WindowSpecification;

public:
    virtual ~Window() = default;

    virtual void Resize(int width, int height) = 0;
    virtual void SwapBuffers() = 0;
    virtual void ProcessEvents(const Event& evt) = 0;

    // Registers the callback every event gets delivered to, translated to Matcha's Event type.
    // Persistent (like SetContextReadyCallback/SetTickCallback below), not per-call: Qt delivers
    // events asynchronously from the viewport widget's own callbacks, which can fire at any time,
    // not just while PumpEvents() runs.
    virtual void SetEventDispatch(std::function<void(const Event&)> dispatch) = 0;

    // Delivers every event that arrived since the last call to the registered dispatch callback.
    // SDL: pumps SDL_PollEvent and translates each one. Qt: a no-op - Qt already delivered input
    // via the viewport widget's own callbacks (using the same registered dispatch) before this is
    // ever called.
    virtual void PumpEvents() = 0;

    // Invoked once the window's GL context is actually current and ready to load function
    // pointers against. SDL: fires synchronously, before the constructor returns (its context is
    // current immediately). Qt: fires later, from QOpenGLWidget::initializeGL(), once Qt's own
    // event loop has actually created the context.
    virtual void SetContextReadyCallback(std::function<void()> callback) = 0;

    // Invoked once per frame to drive the engine loop. SDL: unused - Run()'s own blocking loop
    // calls Application::Tick() directly. Qt: called from the viewport widget's paintGL(), since
    // that's the one place GL calls are guaranteed to target the widget's own framebuffer.
    virtual void SetTickCallback(std::function<void()> callback) = 0;

    [[nodiscard]] virtual int GetWidth() const = 0;
    [[nodiscard]] virtual int GetHeight() const = 0;
    [[nodiscard]] virtual Vector2Int GetCenter() const = 0;
    [[nodiscard]] virtual float GetAspectRatio() const = 0;

    [[nodiscard]] virtual const WindowSpecification& GetWindowSpecification() const = 0;

    [[nodiscard]] virtual bool IsMinimized() const = 0;

    // input is only used by the Qt backend, to wire the viewport widget's key/mouse callbacks to
    // the same Input instance Application owns - ignored by the SDL backend (SDLInput polls
    // global state instead of receiving pushed events).
    [[nodiscard]] static std::unique_ptr<Window> Create(WindowBackend backend, const WindowSpecification& spec = WindowSpecification(), Input* input = nullptr);
};
}  // namespace Matcha
