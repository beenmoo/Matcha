#include "Core/Editor.h"

#include <Core/EntryPoint.h>

namespace Matcha
{
	Application* CreateApplication()
	{
		return new Editor;
	}
}