#include "ScriptSystem.h"
#include "Core/EngineContext.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/NativeScriptComponent.h"
#include "Scene/ScriptableEntity.h"

namespace Matcha
{
void ScriptSystem::Update(Scene& scene, EngineContext& context)
{
    auto view = scene.View<NativeScriptComponent>();

    for (auto handle : view)
    {
        Entity entity(handle, &scene);

        // Skips the whole binding while inactive, including first-time instantiation/OnCreate() -
        // a script on an entity that's never been active shouldn't run side effects (e.g.
        // Flashlight creating its light entity) until it actually goes active.
        if (!IsActiveInHierarchy(entity))
            continue;

        NativeScriptComponent& nsc = entity.GetComponent<NativeScriptComponent>();

        for (auto& binding : nsc.bindings)
        {
            if (!binding.instance)
            {
                binding.instance = binding.instantiateScript();
                binding.instance->m_Entity = entity;
                binding.instance->m_Context = &context;
                binding.instance->OnCreate();
            }

            binding.instance->OnUpdate();
        }
    }
}
}  // namespace Matcha