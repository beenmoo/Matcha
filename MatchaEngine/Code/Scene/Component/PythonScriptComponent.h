#pragma once

// Qt's <QObject> (pulled in transitively by anything deriving from a QWidget, e.g. InspectorPanel.h)
// #defines "slots" to nothing when QT_NO_KEYWORDS isn't set - and CPython's own object.h declares
// "PyType_Slot *slots;" as a literal struct member name, which that macro then mangles into a
// syntax error ("PyType_Slot * ;"). Confirmed by an actual build failure in InspectorPanel.cpp,
// which includes Qt headers (via InspectorPanel.h) before this one. push_macro/pop_macro hides
// the collision only for the duration of pybind11's own include, regardless of what a consumer's
// include order happens to be - the standard fix for this well-known Qt/CPython conflict.
#pragma push_macro("slots")
#undef slots
// pytypes.h, not embed.h: this only needs to hold a py::object, not start/stop an interpreter, so
// it stays as cheap to include as every other component header here.
#include <pybind11/pytypes.h>
#pragma pop_macro("slots")

#include <string>
#include <vector>

namespace Matcha
{
struct PythonScriptComponent
{
    struct Binding
    {
        // Identifies a script as (moduleName, className) - a stable, already-serializable pair,
        // unlike NativeScriptComponent's compiled function pointers. Python's own import system
        // resolves a name to a class for free, so unlike NativeScriptComponent there's no registry
        // of bindings to maintain here - see the ScriptRegistry design discussion this replaced.
        std::string moduleName;
        std::string className;

        // Empty until PythonScriptSystem's first Update() touches this entity - mirrors
        // NativeScriptComponent::Binding::instance's lazy-instantiation timing, and the same
        // reason: on_create()-equivalent side effects shouldn't run until the entity is active.
        //
        // Lifetime hazard: this must never outlive the PythonRuntime that owns the interpreter -
        // destroying a live Python object after Py_Finalize() has run is undefined behavior.
        // Every Scene (and thus every PythonScriptComponent) must be destroyed before
        // Application's PythonRuntime member is - see the ordering comment on
        // Application::m_PythonRuntime.
        pybind11::object instance;
    };

    // Multiple scripts can be bound to the same entity - mirrors NativeScriptComponent's own
    // bindings vector and the reason it existed: a camera entity can carry both Flashlight and
    // CameraController, neither aware of the other, instead of one needing to know about the
    // other (Sandbox.cpp does exactly this).
    std::vector<Binding> bindings;

    void Bind(std::string moduleName, std::string className)
    {
        bindings.push_back({std::move(moduleName), std::move(className), pybind11::object()});
    }
};
}  // namespace Matcha
