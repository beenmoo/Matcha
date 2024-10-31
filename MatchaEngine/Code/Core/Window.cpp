#include "Window.h"
#include "Core.h"
#include "Context.h"

#include <glad/glad.h>
#include <cassert>

namespace Matcha
{
    Window::Window(const WindowSpecification& spec) :
        mWindowSpec(spec)
    {
        InitSDLContext();
        InitWindow();
        InitGLContext();
    }

    void Window::ProcessEvents(const SDL_Event& evt)
    {
        switch (evt.type)
        {
        case SDL_EVENT_WINDOW_RESIZED:
            mWindowSpec.mWidth = evt.window.data1;
            mWindowSpec.mHeight = evt.window.data2;
            glViewport(0, 0, mWindowSpec.mWidth, mWindowSpec.mHeight);
            break;
        default:
            break;
        }
    }

    void Window::Resize(int width, int height)
    {
        SDL_SetWindowSize(mNativeWindow, width, height);
    }

    void Window::SwapBuffers()
    {
        SDL_GL_SwapWindow(mNativeWindow);
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
        return SDL_GetWindowFlags(mNativeWindow) & SDL_WINDOW_MINIMIZED;
    }

    void Window::InitSDLContext()
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            MT_CORE_ERROR(SDL_GetError());

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    }

    void Window::InitWindow()
    {
        mNativeWindow = SDL_CreateWindow(GetWindowSpecification().mTitle.c_str(),
                                         GetWindowSpecification().mWidth,
                                         GetWindowSpecification().mHeight,
                                         GetWindowSpecification().mFlags);

        MT_ASSERT(mNativeWindow, SDL_GetError());
    }

    void Window::InitGLContext()
    {
        mGLContext = SDL_GL_CreateContext(mNativeWindow);
        SDL_GL_MakeCurrent(mNativeWindow, mGLContext);

        int status = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

        MT_ASSERT(status, SDL_GetError());
    }
}