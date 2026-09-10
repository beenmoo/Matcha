#pragma once

#include "Core/Window.h"

#include <SDL3/SDL.h>

namespace Matcha
{
class SDLInput;

class SDLWindow final : public Window
{
public:
    explicit SDLWindow(const WindowSpecification& spec, Input* input);
    ~SDLWindow() override;

    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

    void Resize(int width, int height) override;
    void SwapBuffers() override;
    void ProcessEvents(const Event& evt) override;
    void SetEventDispatch(std::function<void(const Event&)> dispatch) override;
    void PumpEvents() override;
    void SetContextReadyCallback(std::function<void()> callback) override;
    void MakeContextCurrent() override;

    [[nodiscard]] bool IsMinimized() const override;

    // Editor-only escape hatch, mirroring GetViewportWidget()'s role in the old Qt backend: exposes
    // what MatchaEditor's EngineViewportWidget needs but the abstract Window interface has no
    // reason to know about, since SDL is the only backend that's ever hosted this way.
    [[nodiscard]] SDL_Window* GetNativeWindow() const { return m_NativeWindow; }

    // Lets an external host (MatchaEditor's EngineViewportWidget) inject an event it translated
    // itself (from Qt's own key/mouse/wheel callbacks) into the same pipeline PumpEvents() feeds
    // real SDL events through - see SetEventDispatch's own comment for why this needs to be
    // possible outside PumpEvents() at all.
    void DispatchExternalEvent(const Event& evt);

    // Called at the end of every PumpEvents(), after SDL's own SDL_PollEvent loop - lets a host
    // run its own per-Tick polling at the one point guaranteed to land after Input::Update()'s
    // reset and before Update()/OnUpdate()'s read (see Application::Tick()). Unused by Sandbox;
    // MatchaEditor's EngineViewportWidget registers its own PollCursorLock()/PollScrollDelta()
    // equivalents here, the same reasoning the old QtViewportWidget's versions documented.
    void SetExternalPumpCallback(std::function<void()> callback);

private:
    void InitContext();

private:
    SDL_Window* m_NativeWindow = nullptr;
    SDL_GLContext m_GLContext = nullptr;
    SDLInput* m_Input = nullptr;

    std::function<void(const Event&)> m_EventDispatch;
    std::function<void()> m_ExternalPumpCallback;
};
}  // namespace Matcha
