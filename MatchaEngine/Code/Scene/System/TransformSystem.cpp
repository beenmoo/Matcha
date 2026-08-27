#include "TransformSystem.h"

#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"

namespace Matcha
{
void TransformSystem::Update(Scene& scene)
{
    auto view = scene.View<TransformComponent>();

    for (auto handle : view)
    {
        Entity entity(handle, &scene);

        // Non-root entities are reached and written by their ancestor's CascadeHierarchy walk instead.
        if (entity.HasComponent<HierarchyComponent>() && entity.GetComponent<HierarchyComponent>().parent != entt::null)
            continue;

        Matrix4 worldMatrix = entity.GetComponent<TransformComponent>().transform.GetLocalMatrix();
        entity.GetComponent<TransformComponent>().worldMatrix = worldMatrix;

        if (entity.HasComponent<HierarchyComponent>())
            CascadeHierarchy(scene, entity, worldMatrix);
    }
}

void TransformSystem::CascadeHierarchy(Scene& scene, Entity entity, const Matrix4& parentWorldMatrix)
{
    entt::entity childHandle = entity.GetComponent<HierarchyComponent>().firstChild;

    while (childHandle != entt::null)
    {
        Entity child = entity.WithHandle(childHandle);

        Matrix4 worldMatrix = parentWorldMatrix * child.GetComponent<TransformComponent>().transform.GetLocalMatrix();
        child.GetComponent<TransformComponent>().worldMatrix = worldMatrix;

        CascadeHierarchy(scene, child, worldMatrix);

        childHandle = child.GetComponent<HierarchyComponent>().nextSibling;
    }
}
}  // namespace Matcha
