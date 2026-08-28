#pragma once

#include "Core/Window.h"

#include <SDL3/SDL.h>

namespace Matcha
{
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
    void SetTickCallback(std::function<void()> callback) override;

    [[nodiscard]] int GetWidth() const override;
    [[nodiscard]] int GetHeight() const override;
    [[nodiscard]] Vector2Int GetCenter() const override;
    [[nodiscard]] float GetAspectRatio() const override;

    [[nodiscard]] const WindowSpecification& GetWindowSpecification() const override;

    [[nodiscard]] bool IsMinimized() const override;

private:
    void InitContext();

private:
    SDL_Window* m_NativeWindow = nullptr;
    SDL_GLContext m_GLContext = nullptr;

    WindowSpecification m_WindowSpec;
    std::function<void(const Event&)> m_EventDispatch;
};
}  // namespace Matcha
