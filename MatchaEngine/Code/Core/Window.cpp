#include "Window.h"
#include "Assert.h"
#include "SDL/SDLWindow.h"

#ifdef MT_ENABLE_QT_BACKEND
#include "Qt/QtWindow.h"
#endif

namespace Matcha
{
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
