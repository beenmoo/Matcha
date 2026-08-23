#pragma once

#include "Core/Assert.h"
#include "Core/Logger.h"
#include "Context.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"

int main(int argc, char** argv);

namespace Matcha
{
struct ApplicationCommandLineArgs
{
    int mCount = 0;
    char** mArgs = nullptr;

    [[nodiscard]] const char* operator[](int index) const
    {
        MT_ASSERT(index < mCount && index >= 0, "Out of range");

        return mArgs[index];
    }
};

struct ApplicationSpecification
{
    std::string mTitle = "Application";
    std::string mWorkingDirectory;
    ApplicationCommandLineArgs mCommandLineArgs;
};

class Application
{
public:
    using ApplicationCommandLineArgs = Matcha::ApplicationCommandLineArgs;
    using ApplicationSpecification = Matcha::ApplicationSpecification;

public:
    Application(const ApplicationSpecification& spec = ApplicationSpecification());
    virtual ~Application();

    void Run();
    void Quit();

protected:
    template <typename Self>
    [[nodiscard]] auto& GetContext(this Self& self)
    {
        return self.mContext;
    }

private:
    void Update();
    void PollEvents();
    void LogContext();

private:
    ApplicationSpecification mAppSpec;

    Input mInput;
    Logger mLogger;
    Time mTime;
    Window mWindow;
    Context mContext;

    bool mIsRunning = false;
};

[[nodiscard]] Application* CreateApplication(const Application::ApplicationCommandLineArgs& args);
}  // namespace Matcha