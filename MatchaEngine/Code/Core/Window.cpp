#include "Window.h"
#include "Assert.h"
#include "SDL/SDLWindow.h"

#include <glad/glad.h>

namespace Matcha
{
Window::Window(const WindowSpecification& spec)
    : m_WindowSpec(spec)
{
}

void Window::HandleResizeEvent(const Event& evt)
{
    if (evt.type != EventType::WindowResized)
        return;

    m_WindowSpec.m_Width = evt.width;
    m_WindowSpec.m_Height = evt.height;
    glViewport(0, 0, m_WindowSpec.m_Width, m_WindowSpec.m_Height);
}

std::unique_ptr<Window> Window::Create(WindowBackend backend, const WindowSpecification& spec, Input* input)
{
    switch (backend)
    {
    case WindowBackend::SDL:
        return std::make_unique<SDLWindow>(spec, input);
    default:
        MT_ASSERT(false, "Unknown window backend");
        return nullptr;
    }
}
}  // namespace Matcha
