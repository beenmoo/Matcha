#include "ScriptSystem.h"
#include "Core/EngineContext.h"
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