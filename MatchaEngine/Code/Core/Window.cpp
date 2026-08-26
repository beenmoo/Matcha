#include "Window.h"
#include "Assert.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <cassert>

namespace Matcha
{
struct Window::Impl
{
    SDL_Window* mNativeWindow = nullptr;
    SDL_GLContext mGLContext = nullptr;
};

Window::Window(const WindowSpecification& spec) : mImpl(std::make_unique<Impl>()),
                                                  mWindowSpec(spec)
{
    InitContext();
}

Window::~Window() = default;
Window::Window(Window&&) noexcept = default;
Window& Window::operator=(Window&&) noexcept = default;

void Window::ProcessEvents(const Event& evt)
{
    switch (evt.type)
    {
    case EventType::WindowResized:
        mWindowSpec.mWidth = evt.width;
        mWindowSpec.mHeight = evt.height;
        glViewport(0, 0, mWindowSpec.mWidth, mWindowSpec.mHeight);
        break;
    default:
        break;
    }
}

void Window::Resize(int width, int height)
{
    SDL_SetWindowSize(mImpl->mNativeWindow, width, height);
}

void Window::SwapBuffers()
{
    SDL_GL_SwapWindow(mImpl->mNativeWindow);
}

int Window::GetWidth() const
{
    return mWindowSpec.mWidth;
}

int Window::GetHeight() const
{
    return mWindowSpec.mHeight;
}

Vector2Int Window::GetCenter() const
{
    return Vector2Int(mWindowSpec.mWidth / 2, mWindowSpec.mHeight / 2);
}

float Window::GetAspectRatio() const
{
    return static_cast<float>(mWindowSpec.mWidth) / mWindowSpec.mHeight;
}

const Window::WindowSpecification& Window::GetWindowSpecification() const
{
    return mWindowSpec;
}

bool Window::IsMinimized() const
{
    return SDL_GetWindowFlags(mImpl->mNativeWindow) & SDL_WINDOW_MINIMIZED;
}

void Window::InitContext()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        MT_CORE_ERROR("{}", SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    uint32_t flags = SDL_WINDOW_OPENGL;

    if (mWindowSpec.mResizable)
        flags |= SDL_WINDOW_RESIZABLE;

    mImpl->mNativeWindow = SDL_CreateWindow(GetWindowSpecification().mTitle.c_str(),
                                            GetWindowSpecification().mWidth,
                                            GetWindowSpecification().mHeight,
                                            flags);

    MT_ASSERT(mImpl->mNativeWindow, SDL_GetError());

    if (mWindowSpec.mPosition)
        SDL_SetWindowPosition(mImpl->mNativeWindow, mWindowSpec.mPosition->x, mWindowSpec.mPosition->y);
    else
        SDL_SetWindowPosition(mImpl->mNativeWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    mImpl->mGLContext = SDL_GL_CreateContext(mImpl->mNativeWindow);
    SDL_GL_MakeCurrent(mImpl->mNativeWindow, mImpl->mGLContext);

    int status = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    MT_ASSERT(status, SDL_GetError());
}
}  // namespace Matcha