#pragma once

#include "Scene/Entity.h"
#include "Scene/Scene.h"

namespace Matcha
{
// Intrusive doubly-linked list of siblings, storing raw entt::entity rather than our Entity
// wrapper: fixed size, no heap allocation, O(1) attach/detach regardless of sibling count, and
// no per-field Scene* bloat (every field already belongs to the same scene as the entity that
// owns this component, so there's nothing to gain from four Entity-sized fields here).
struct HierarchyComponent
{
    size_t childrenCount = 0;
    entt::entity parent = entt::null;
    entt::entity firstChild = entt::null;
    entt::entity prevSibling = entt::null;
    entt::entity nextSibling = entt::null;
};

// Reparents `child` under `newParent`, relinking siblings on both the old and new parent.
// Pass a default-constructed (invalid) Entity as newParent to detach `child` to the root.
// Adds a HierarchyComponent to either entity if it doesn't already have one.
//
// Assumes SetParent is the only way entities get linked: any handle reachable through a
// parent/firstChild/prevSibling/nextSibling field is guaranteed to already have its own
// HierarchyComponent.
inline void SetParent(Entity child, Entity newParent)
{
    // Both AddComponent calls happen before any reference into the HierarchyComponent pool is
    // taken below: emplacing a new component can reallocate the pool and invalidate references
    // obtained from an earlier GetComponent<HierarchyComponent>() call.
    if (!child.HasComponent<HierarchyComponent>())
        child.AddComponent<HierarchyComponent>();

    if (newParent.IsValid() && !newParent.HasComponent<HierarchyComponent>())
        newParent.AddComponent<HierarchyComponent>();

    HierarchyComponent& childNode = child.GetComponent<HierarchyComponent>();

    // Unlink from the current parent/siblings, if any.
    if (childNode.prevSibling != entt::null)
        child.WithHandle(childNode.prevSibling).GetComponent<HierarchyComponent>().nextSibling = childNode.nextSibling;
    else if (childNode.parent != entt::null)
        child.WithHandle(childNode.parent).GetComponent<HierarchyComponent>().firstChild = childNode.nextSibling;

    if (childNode.nextSibling != entt::null)
        child.WithHandle(childNode.nextSibling).GetComponent<HierarchyComponent>().prevSibling = childNode.prevSibling;

    if (childNode.parent != entt::null)
        --child.WithHandle(childNode.parent).GetComponent<HierarchyComponent>().childrenCount;

    childNode.parent = entt::null;
    childNode.prevSibling = entt::null;
    childNode.nextSibling = entt::null;

    // Attach to the new parent as its new first child.
    if (newParent.IsValid())
    {
        HierarchyComponent& parentNode = newParent.GetComponent<HierarchyComponent>();

        childNode.parent = newParent.GetHandle();
        childNode.nextSibling = parentNode.firstChild;

        if (parentNode.firstChild != entt::null)
            newParent.WithHandle(parentNode.firstChild).GetComponent<HierarchyComponent>().prevSibling = child.GetHandle();

        parentNode.firstChild = child.GetHandle();
        ++parentNode.childrenCount;
    }
}

namespace detail
{
// Destroys `entity` and all of its descendants. Does not unlink `entity` from its own parent's
// child list: the caller (DestroyEntityRecursive) is responsible for that, and only needs to do
// it once, at the root of the subtree being destroyed.
inline void DestroySubtree(Scene& scene, Entity entity)
{
    if (entity.HasComponent<HierarchyComponent>())
    {
        entt::entity childHandle = entity.GetComponent<HierarchyComponent>().firstChild;

        while (childHandle != entt::null)
        {
            Entity child = entity.WithHandle(childHandle);

            // Capture the next sibling before destroying this child, since destroying it also
            // destroys its HierarchyComponent (and thus invalidates childHandle's own data).
            childHandle = child.GetComponent<HierarchyComponent>().nextSibling;

            DestroySubtree(scene, child);
        }
    }

    scene.DestroyEntity(entity);
}
}  // namespace detail

// Destroys `entity` and, recursively, all of its descendants (as linked via HierarchyComponent).
// If `entity` has a parent, it's unlinked from that parent's child list first, so the parent
// doesn't retain a dangling handle to a now-destroyed entity.
inline void DestroyEntityRecursive(Scene& scene, Entity entity)
{
    if (entity.HasComponent<HierarchyComponent>())
        SetParent(entity, Entity());

    detail::DestroySubtree(scene, entity);
}
}  // namespace Matcha
