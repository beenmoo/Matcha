#include "Window.h"
#include "Core.h"
#include "Context.h"

#include <glad/glad.h>
#include <cassert>

namespace Matcha
{
    Window::Window(Context& ctx, const WindowSpecification& spec) :
        mContext(ctx),
        mWindowSpec(spec)
    {}

    void Window::ProcessEvents(const SDL_Event& evt)
    {
        switch (evt.type)
        {
        case SDL_WINDOWEVENT:
            switch (evt.window.event)
            {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                mWindowSpec.mWidth = evt.window.data1;
                mWindowSpec.mHeight = evt.window.data2;
                glViewport(0, 0, mWindowSpec.mWidth, mWindowSpec.mHeight);
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    void Window::Resize(int width, int height)
    {
        SDL_SetWindowSize(mContext.GetNativeWindow(), width, height);
    }

    void Window::SwapBuffers()
    {
        SDL_GL_SwapWindow(mContext.GetNativeWindow());
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
        return SDL_GetWindowFlags(mContext.GetNativeWindow()) & SDL_WINDOW_MINIMIZED;
    }
}