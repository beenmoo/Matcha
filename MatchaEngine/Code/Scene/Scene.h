#pragma once

#include "Entity.h"

namespace Matcha
{
class Scene
{
public:
    Scene() = default;

    [[nodiscard]] Entity CreateEntity();
    void DestroyEntity(Entity entity);

    template <typename... Components>
    [[nodiscard]] auto View()
    {
        return m_Registry.view<Components...>();
    }

private:
    friend class Entity;

    entt::registry m_Registry;
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
