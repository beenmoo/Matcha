#include "PythonScriptSystem.h"
#include "Core/Input.h"
#include "Core/Logger.h"
#include "Core/Time.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/PythonScriptComponent.h"
#include "Scripting/PythonRuntime.h"

namespace Matcha
{
void PythonScriptSystem::Update(Scene& scene, Input& input, Time& time, PythonRuntime& pythonRuntime)
{
    auto view = scene.View<PythonScriptComponent>();

    for (auto handle : view)
    {
        Entity entity(handle, &scene);

        // Skips every binding on this entity (including first-time instantiation/on_create)
        // while inactive - a script on an entity that's never been active shouldn't run side
        // effects (e.g. Flashlight creating its light entity) until it actually goes active.
        //
        // Deliberately the uncached walk: this runs from Application::Update(), before Render()
        // runs TransformSystem, so TransformComponent::activeInHierarchy still holds last frame's
        // answer here. A one-frame lag is harmless for skipping a draw; it isn't for gating
        // whether a script instantiates and runs side effects.
        if (!IsActiveInHierarchy(entity))
            continue;

        PythonScriptComponent& script = entity.GetComponent<PythonScriptComponent>();

        for (PythonScriptComponent::Binding& binding : script.bindings)
        {
            try
            {
                if (!binding.instance || binding.instance.is_none())
                {
                    py::module_ module = pythonRuntime.LoadScriptModule(binding.moduleName);
                    if (!module)
                        continue;

                    py::object cls = module.attr(binding.className.c_str());
                    binding.instance = cls();
                    binding.instance.attr("entity") = py::cast(entity);
                    binding.instance.attr("input") = py::cast(&input);
                    binding.instance.attr("time") = py::cast(&time);
                    binding.instance.attr("scene") = py::cast(&scene);

                    if (py::hasattr(binding.instance, "on_create"))
                        binding.instance.attr("on_create")();
                }

                if (py::hasattr(binding.instance, "on_update"))
                    binding.instance.attr("on_update")();
            }
            catch (const py::error_already_set& e)
            {
                // A broken script is untrusted input the same way a bad shader source file is -
                // log and skip this binding for the frame rather than let a Python exception
                // unwind into the engine's own frame loop.
                MT_CORE_ERROR("Python script '{}.{}' raised: {}", binding.moduleName, binding.className, e.what());
            }
        }
    }
}
}  // namespace Matcha
