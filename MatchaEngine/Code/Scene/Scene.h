#pragma once

#include "Entity.h"
#include "Utility/UUID.h"

#include <functional>
#include <string>
#include <vector>

namespace Matcha
{
class Scene
{
public:
    Scene() = default;

    [[nodiscard]] Entity CreateEntity(std::string name = "");
    [[nodiscard]] Entity CreateEntity(UUID id, std::string name = "");

    // notify=false is for internal use by HierarchyComponent.h's recursive subtree destruction,
    // which destroys several entities in a row while ancestor HierarchyComponents still hold
    // dangling links into the entities being torn down - notifying mid-teardown would let a
    // scene-changed observer (the Scene Hierarchy panel) walk into one of those dangling links.
    void DestroyEntity(Entity entity, bool notify = true);

    // Every entity with no parent (no HierarchyComponent, or one with parent == entt::null).
    // Unlike View<Components...>(), which filters by component set, this walks every live entity -
    // it's what tree-building UI (the Scene Hierarchy panel) needs to find its starting points.
    [[nodiscard]] std::vector<Entity> GetRootEntities();

    // Multi-subscriber: both the Scene Hierarchy panel (rebuilds its tree) and the Inspector
    // panel (re-checks whether its selected entity is still valid) observe this.
    void AddOnSceneChanged(std::function<void()> callback);
    void NotifyChanged();

    template <typename... Components>
    [[nodiscard]] auto View()
    {
        return m_Registry.view<Components...>();
    }

private:
    friend class Entity;

    entt::registry m_Registry;
    std::vector<std::function<void()>> m_OnSceneChanged;
};

// Entity's members that need Scene to be a complete type are defined here rather than in
// Entity.h (which only forward-declares Scene). IsValid() is marked inline since, unlike the
// templates below, it isn't implicitly inline and this header is included from multiple TUs.
inline bool Entity::IsValid() const
{
    return m_Scene != nullptr && m_Scene->m_Registry.valid(m_Handle);
}

template <typename T, typename... Args>
T& Entity::AddComponent(Args&&... args)
{
    return m_Scene->m_Registry.emplace<T>(m_Handle, std::forward<Args>(args)...);
}

template <typename T>
T& Entity::GetComponent()
{
    return m_Scene->m_Registry.get<T>(m_Handle);
}

template <typename T>
const T& Entity::GetComponent() const
{
    return m_Scene->m_Registry.get<T>(m_Handle);
}

template <typename T>
bool Entity::HasComponent() const
{
    return m_Scene->m_Registry.all_of<T>(m_Handle);
}

template <typename T>
void Entity::RemoveComponent()
{
    m_Scene->m_Registry.remove<T>(m_Handle);
}
}  // namespace Matcha
