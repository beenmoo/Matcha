#include "Context.h"
#include "Application.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"

namespace Matcha
{
Context::Context(Application& application,
                 Input& input,
                 Time& time,
                 Window& window) : mApplication(application),
                                   mInput(input),
                                   mTime(time),
                                   mWindow(window)
{
}

Application& Context::GetApplication()
{
    return mApplication;
}

const Application& Context::GetApplication() const
{
    return mApplication;
}

Input& Context::GetInput()
{
    return mInput;
}

const Input& Context::GetInput() const
{
    return mInput;
}

Time& Context::GetTime()
{
    return mTime;
}

const Time& Context::GetTime() const
{
    return mTime;
}

Window& Context::GetWindow()
{
    return mWindow;
}

const Window& Context::GetWindow() const
{
    return mWindow;
}
}  // namespace Matcha