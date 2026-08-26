#include "Core/Editor.h"

#include <Core/EntryPoint.h>

namespace Matcha
{
Application* CreateApplication(const Application::ApplicationCommandLineArgs& args)
{
    Application::ApplicationSpecification spec;
    spec.m_Title = "Hazelnut";
    spec.m_CommandLineArgs = args;

    return new Editor(spec);
}
}  // namespace Matcha