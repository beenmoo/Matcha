#pragma once

// pytypes.h, not embed.h: this only needs to hold a py::object, not start/stop an interpreter, so
// it stays as cheap to include as every other component header here.
#include <pybind11/pytypes.h>

#include <string>

namespace Matcha
{
// A behavior script backed by a Python class, identified by (moduleName, className) - a stable,
// already-serializable pair, unlike NativeScriptComponent's compiled function pointers. Python's
// own import system resolves a name to a class for free, so unlike NativeScriptComponent there's
// no registry of bindings to maintain here - see the ScriptRegistry design discussion this
// component's design replaced.
struct PythonScriptComponent
{
    std::string moduleName;
    std::string className;

    // Empty until PythonScriptSystem's first Update() touches this entity - mirrors
    // NativeScriptComponent::Binding::instance's lazy-instantiation timing, and the same reason:
    // on_create()-equivalent side effects shouldn't run until the entity is actually active.
    //
    // Lifetime hazard: this must never outlive the PythonRuntime that owns the interpreter -
    // destroying a live Python object after Py_Finalize() has run is undefined behavior. Every
    // Scene (and thus every PythonScriptComponent) must be destroyed before Application's
    // PythonRuntime member is - see the ordering comment on Application::m_PythonRuntime.
    pybind11::object instance;
};
}  // namespace Matcha
