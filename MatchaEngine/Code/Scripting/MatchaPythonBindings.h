#pragma once

namespace Matcha
{
// Does nothing itself - exists purely so PythonRuntime's constructor has a real symbol to call.
// MatchaEngine links as a static library, and a .obj whose only content is a PYBIND11_EMBEDDED_MODULE
// static initializer (nothing else in it referenced by anything) is exactly what a linker is free
// to drop from the final binary, silently losing the "import matcha_engine" registration with it.
// Calling this forces MatchaPythonBindings.cpp's whole translation unit - and therefore its static
// initializer - into the link.
void ForceLinkMatchaPythonBindings();
}  // namespace Matcha
