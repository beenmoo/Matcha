#include "Core/Sandbox.h"

#include <Core/EntryPoint.h>

namespace Matcha
{
	Application* CreateApplication(const Application::ApplicationCommandLineArgs& args)
	{
		Application::ApplicationSpecification spec;
		spec.mTitle = "Matcha";
		spec.mCommandLineArgs = args;

		return new Sandbox(spec);
	}
}