#pragma once

#include "Core.h"
#include "Context.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"

int main(int argc, char** argv);

namespace Matcha
{
    class Application
    {
    public:
        struct ApplicationCommandLineArgs
        {
            int mCount = 0;
            char** mArgs = nullptr;

            const char* operator[](int index) const
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

    public:
        Application(const ApplicationSpecification& spec = ApplicationSpecification());
        virtual ~Application();

        void Run();
        void Quit();

    protected:
        Context& GetContext();
        const Context& GetContext() const;

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

    Application* CreateApplication(const Application::ApplicationCommandLineArgs& args);
}