#pragma once

#include "Application.h"

extern Matcha::Application* Matcha::CreateApplication(const Application::ApplicationCommandLineArgs& args);

int main(int argc, char** argv)
{
    auto app = Matcha::CreateApplication({argc, argv});

    app->Run();

    delete app;

    return 0;
}