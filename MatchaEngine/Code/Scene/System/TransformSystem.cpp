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

    // Cleared up front, then set true only for entities the cascade below actually reaches. An
    // entity skipped for being inactive (or sitting under one that is) is simply never visited, so
    // without this pass its flag would keep whatever value it held when it was last active.
    for (auto handle : view)
        view.template get<TransformComponent>(handle).activeInHierarchy = false;

    for (auto handle : view)
    {
        Entity entity(handle, &scene);

        // Non-root entities are reached and written by their ancestor's CascadeHierarchy walk instead.
        if (entity.HasComponent<HierarchyComponent>() && entity.GetComponent<HierarchyComponent>().GetParent() != entt::null)
            continue;

        // A root has no parent to inherit inactivity from, so its own flag is the whole story -
        // unlike CascadeHierarchy below, which relies on this check having already propagated
        // down from every active ancestor above it.
        if (!IsActiveSelf(entity))
            continue;

        TransformComponent& transformComponent = entity.GetComponent<TransformComponent>();

        Matrix4 worldMatrix = transformComponent.transform.GetLocalMatrix();
        transformComponent.worldMatrix = worldMatrix;
        transformComponent.activeInHierarchy = true;

        if (entity.HasComponent<HierarchyComponent>())
            CascadeHierarchy(scene, entity, worldMatrix);
    }
}

void TransformSystem::CascadeHierarchy(Scene& scene, Entity entity, const Matrix4& parentWorldMatrix)
{
    entt::entity childHandle = entity.GetComponent<HierarchyComponent>().GetFirstChild();

    while (childHandle != entt::null)
    {
        Entity child = entity.WithHandle(childHandle);

        // Skipping an inactive child here - rather than checking IsActiveInHierarchy() per-entity
        // in every system - already propagates to its descendants for free: they're simply never
        // reached, since this recursion is the only thing that visits them.
        if (IsActiveSelf(child))
        {
            TransformComponent& childTransform = child.GetComponent<TransformComponent>();

            Matrix4 worldMatrix = parentWorldMatrix * childTransform.transform.GetLocalMatrix();
            childTransform.worldMatrix = worldMatrix;

            // Reached only via an active parent (this recursion is the sole path here), so being
            // active itself is the whole remaining condition for being active in the hierarchy.
            childTransform.activeInHierarchy = true;

            CascadeHierarchy(scene, child, worldMatrix);
        }

        childHandle = child.GetComponent<HierarchyComponent>().GetNextSibling();
    }
}
}  // namespace Matcha
