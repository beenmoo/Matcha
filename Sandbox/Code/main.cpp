#include "Core/Sandbox.h"

#include <Core/EntryPoint.h>

namespace Matcha
{
Application* CreateApplication(const Application::ApplicationCommandLineArgs& args)
{
    Application::ApplicationSpecification spec;
    spec.m_Title = "Matcha";
    spec.m_CommandLineArgs = args;

    return new Sandbox(spec);
}
}  // namespace Matcha