#include "TransformSystem.h"

#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"

namespace Matcha
{
namespace
{
bool IsActiveSelf(Entity entity)
{
    return !entity.HasComponent<TagComponent>() || entity.GetComponent<TagComponent>().isActive;
}
}  // namespace

void TransformSystem::Update(Scene& scene)
{
    auto view = scene.View<TransformComponent>();

    for (auto handle : view)
    {
        Entity entity(handle, &scene);

        // Non-root entities are reached and written by their ancestor's CascadeHierarchy walk instead.
        if (entity.HasComponent<HierarchyComponent>() && entity.GetComponent<HierarchyComponent>().parent != entt::null)
            continue;

        // A root has no parent to inherit inactivity from, so its own flag is the whole story -
        // unlike CascadeHierarchy below, which relies on this check having already propagated
        // down from every active ancestor above it.
        if (!IsActiveSelf(entity))
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

        // Skipping an inactive child here - rather than checking IsActiveInHierarchy() per-entity
        // in every system - already propagates to its descendants for free: they're simply never
        // reached, since this recursion is the only thing that visits them.
        if (IsActiveSelf(child))
        {
            Matrix4 worldMatrix = parentWorldMatrix * child.GetComponent<TransformComponent>().transform.GetLocalMatrix();
            child.GetComponent<TransformComponent>().worldMatrix = worldMatrix;

            CascadeHierarchy(scene, child, worldMatrix);
        }

        childHandle = child.GetComponent<HierarchyComponent>().nextSibling;
    }
}
}  // namespace Matcha
