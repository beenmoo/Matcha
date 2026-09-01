#include "Core/Editor.h"

#include <Core/EntryPoint.h>

namespace Matcha
{
	Application* CreateApplication(const Application::ApplicationCommandLineArgs& args)
	{
		Application::ApplicationSpecification spec;
		spec.mTitle = "Matcha Editor";
		spec.mCommandLineArgs = args;

		return new Editor(spec);
	}
}