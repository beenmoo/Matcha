#include "PythonScriptSystem.h"
#include "Core/EngineContext.h"
#include "Core/Logger.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/PythonScriptComponent.h"
#include "Scripting/PythonRuntime.h"

namespace Matcha
{
void PythonScriptSystem::Update(Scene& scene, EngineContext& context)
{
    auto view = scene.View<PythonScriptComponent>();

    for (auto handle : view)
    {
        Entity entity(handle, &scene);

        // Same reasoning as ScriptSystem::Update: skip the whole binding (including first-time
        // instantiation/on_create) while inactive, and use the uncached walk since this runs
        // before TransformSystem for the frame - see that comment for the full explanation.
        if (!IsActiveInHierarchy(entity))
            continue;

        PythonScriptComponent& script = entity.GetComponent<PythonScriptComponent>();

        try
        {
            if (!script.instance || script.instance.is_none())
            {
                py::module_ module = context.GetPythonRuntime().LoadScriptModule(script.moduleName);
                if (!module)
                    continue;

                py::object cls = module.attr(script.className.c_str());
                script.instance = cls();
                script.instance.attr("entity") = py::cast(entity);

                if (py::hasattr(script.instance, "on_create"))
                    script.instance.attr("on_create")();
            }

            if (py::hasattr(script.instance, "on_update"))
                script.instance.attr("on_update")();
        }
        catch (const py::error_already_set& e)
        {
            // A broken script is untrusted input the same way a bad shader source file is - log
            // and skip this entity's binding for the frame rather than let a Python exception
            // unwind into the engine's own frame loop.
            MT_CORE_ERROR("Python script '{}.{}' raised: {}", script.moduleName, script.className, e.what());
        }
    }
}
}  // namespace Matcha
