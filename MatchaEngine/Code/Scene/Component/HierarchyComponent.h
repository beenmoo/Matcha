#pragma once

#include "Scene/Component/TagComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include <vector>

namespace Matcha
{
// Intrusive doubly-linked list of siblings, storing raw entt::entity rather than our Entity
// wrapper: fixed size, no heap allocation, O(1) attach/detach regardless of sibling count, and
// no per-field Scene* bloat (every field already belongs to the same scene as the entity that
// owns this component, so there's nothing to gain from four Entity-sized fields here).
struct HierarchyComponent
{
    // Read-only for everyone: every system that walks the hierarchy (TransformSystem,
    // SceneSerializer, the Scene Hierarchy panel, ...) only ever needs to query these links, never
    // set them directly - SetParent below is the sole place that may mutate them, since it's the
    // only code that knows how to keep the doubly-linked list consistent.
    [[nodiscard]] size_t GetChildrenCount() const { return childrenCount; }
    [[nodiscard]] entt::entity GetParent() const { return parent; }
    [[nodiscard]] entt::entity GetFirstChild() const { return firstChild; }
    [[nodiscard]] entt::entity GetPrevSibling() const { return prevSibling; }
    [[nodiscard]] entt::entity GetNextSibling() const { return nextSibling; }

private:
    friend void SetParent(Entity child, Entity newParent);

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

    child.GetScene()->NotifyChanged();
}

// True unless `entity` or any ancestor (walking up via HierarchyComponent::parent) has
// TagComponent::isActive set false - i.e. "activeInHierarchy", not just the entity's own flag,
// since disabling a parent is expected to disable its whole subtree. An entity with no
// TagComponent at all counts as active, matching the "no TagComponent = default" precedent used
// elsewhere (e.g. the Scene Hierarchy panel's fallback label).
//
// Systems that iterate hierarchy order top-down already (TransformSystem's cascade) don't need
// this - checking each child's own isActive as it's reached already propagates inactivity to
// descendants "for free", without redundantly re-walking ancestors already known to be active.
// This is for systems that iterate flat (RenderSystem, LightSystem, CameraSystem, ScriptSystem),
// which have no such ordering guarantee.
inline bool IsActiveInHierarchy(Entity entity)
{
    while (entity.IsValid())
    {
        if (entity.HasComponent<TagComponent>() && !entity.GetComponent<TagComponent>().isActive)
            return false;

        entity = entity.HasComponent<HierarchyComponent>() ? entity.WithHandle(entity.GetComponent<HierarchyComponent>().GetParent()) : Entity();
    }

    return true;
}

// Appends `entity` and every descendant beneath it to `out`, parents before children. Shares the
// traversal shape of detail::DestroySubtree below, but collects instead of destroying - for
// callers that need the whole subtree as data first (copying it to a clipboard, snapshotting it
// before a delete so the delete can be undone).
inline void CollectSubtree(Entity entity, std::vector<Entity>& out)
{
    out.push_back(entity);

    if (entity.HasComponent<HierarchyComponent>())
    {
        entt::entity childHandle = entity.GetComponent<HierarchyComponent>().GetFirstChild();

        while (childHandle != entt::null)
        {
            Entity child = entity.WithHandle(childHandle);
            CollectSubtree(child, out);
            childHandle = child.GetComponent<HierarchyComponent>().GetNextSibling();
        }
    }
}

// The cached form of IsActiveInHierarchy(), for systems running after TransformSystem has already
// computed it for this frame (every system does - see Application::RegisterSystems). Prefer reading
// TransformComponent::activeInHierarchy directly where the system's view already includes the
// transform; this exists for views that don't (CameraSystem, ScriptSystem).
//
// Falls back to the uncached walk for an entity with no TransformComponent - Scene::CreateEntity
// always adds one, so that only happens for an entity assembled by hand.
inline bool IsActiveInHierarchyCached(Entity entity)
{
    if (entity.HasComponent<TransformComponent>())
        return entity.GetComponent<TransformComponent>().activeInHierarchy;

    return IsActiveInHierarchy(entity);
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
        entt::entity childHandle = entity.GetComponent<HierarchyComponent>().GetFirstChild();

        while (childHandle != entt::null)
        {
            Entity child = entity.WithHandle(childHandle);

            // Capture the next sibling before destroying this child, since destroying it also
            // destroys its HierarchyComponent (and thus invalidates childHandle's own data).
            childHandle = child.GetComponent<HierarchyComponent>().GetNextSibling();

            DestroySubtree(scene, child);
        }
    }

    // Silent: ancestors still up the stack hold HierarchyComponent links into entities being
    // destroyed here, so notifying now would let an observer (e.g. the Scene Hierarchy panel)
    // walk into a dangling handle before the whole subtree finishes coming down. The caller
    // (DestroyEntityRecursive) notifies once, after everything below is actually gone.
    scene.DestroyEntity(entity, false);
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

    scene.NotifyChanged();
}
}  // namespace Matcha
