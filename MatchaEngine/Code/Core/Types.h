#pragma once

#include <string>
#include <string_view>
#include <SDL3/SDL.h>

namespace Matcha
{
    using String = std::string;
    using Event = SDL_Event;
    using GLContext = SDL_GLContext;
    using GLWindow = SDL_Window;
    using uint64 = Uint64;
}