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

        if (!nsc.instance)
        {
            nsc.instance = nsc.instantiateScript();
            nsc.instance->m_Entity = entity;
            nsc.instance->m_Context = &context;
            nsc.instance->OnCreate();
        }

        nsc.instance->OnUpdate();
    }
}
}  // namespace Matcha