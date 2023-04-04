#include "Sandbox.h"

#include <Core/EntryPoint.h>

namespace Matcha
{
	Application* CreateApplication(const Application::ApplicationCommandLineArgs& args)
	{
		Application::ApplicationSpecification spec;
		spec.mTitle = "Hazelnut";
		spec.mCommandLineArgs = args;

		return new Sandbox(spec);
	}
}