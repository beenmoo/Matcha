#pragma once

int main(int argc, char** argv);

namespace Matcha
{
    class Application
    {
    public:
        virtual ~Application() = default;

    private:
        void Run();

    private:
        bool mIsRunning = false;
    };

    Application* CreateApplication();
}