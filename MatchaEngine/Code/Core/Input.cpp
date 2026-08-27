#include "Input.h"
#include "Assert.h"
#include "SDL/SDLInput.h"

#ifdef MT_ENABLE_QT_BACKEND
#include "Qt/QtInput.h"
#endif

namespace Matcha
{
std::unique_ptr<Input> Input::Create(WindowBackend backend)
{
    switch (backend)
    {
    case WindowBackend::SDL:
        return std::make_unique<SDLInput>();
#ifdef MT_ENABLE_QT_BACKEND
    case WindowBackend::Qt:
        return std::make_unique<QtInput>();
#endif
    default:
        MT_ASSERT(false, "Input backend not supported (was MatchaEngine built with BUILD_QT_BACKEND?)");
        return nullptr;
    }
}
}  // namespace Matcha
