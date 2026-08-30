#include "Window.h"
#include "Assert.h"
#include "SDL/SDLWindow.h"

#ifdef MT_ENABLE_QT_BACKEND
#include "Qt/QtWindow.h"
#endif

#include <glad/glad.h>

namespace Matcha
{
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
#ifdef MT_ENABLE_QT_BACKEND
    case WindowBackend::Qt:
        return std::make_unique<QtWindow>(spec, input);
#endif
    default:
        MT_ASSERT(false, "Window backend not supported (was MatchaEngine built with BUILD_QT_BACKEND?)");
        return nullptr;
    }
}
}  // namespace Matcha
