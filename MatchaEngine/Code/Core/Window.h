#pragma once

#include "Math/Vector.h"
#include "Core/Event.h"

#include <string>
#include <memory>
#include <optional>

namespace Matcha
{
class Context;

struct WindowSpecification
{
    std::string m_Title = "Application";
    int m_Width = 1280;
    int m_Height = 720;
    std::optional<Vector2Int> m_Position;
    bool m_Resizable = true;
};

class Window
{
public:
    using WindowSpecification = Matcha::WindowSpecification;

public:
    Window(const WindowSpecification& spec = WindowSpecification());
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;

    void Resize(int width, int height);
    void SwapBuffers();
    void ProcessEvents(const Event& evt);

    [[nodiscard]] int GetWidth() const;
    [[nodiscard]] int GetHeight() const;
    [[nodiscard]] Vector2Int GetCenter() const;
    [[nodiscard]] float GetAspectRatio() const;

    [[nodiscard]] const WindowSpecification& GetWindowSpecification() const;

    [[nodiscard]] bool IsMinimized() const;

private:
    void InitContext();

private:
    struct Impl;
    std::unique_ptr<Impl> m_Impl;

    WindowSpecification m_WindowSpec;
    bool m_IsOpen = false;
};
}  // namespace Matcha