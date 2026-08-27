#pragma once

#include <entt/entt.hpp>

namespace Matcha
{
class Scene;

class Entity
{
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene)
        : m_Handle(handle),
          m_Scene(scene)
    {
    }

    // Defined in Scene.h: the bodies need Scene to be a complete type.
    template <typename T, typename... Args>
    T& AddComponent(Args&&... args);

    template <typename T>
    [[nodiscard]] T& GetComponent();

    template <typename T>
    [[nodiscard]] const T& GetComponent() const;

    template <typename T>
    [[nodiscard]] bool HasComponent() const;

    template <typename T>
    void RemoveComponent();

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] entt::entity GetHandle() const
    {
        return m_Handle;
    }

    // Wraps a raw handle (e.g. one pulled out of a component) as an Entity in this same scene.
    [[nodiscard]] Entity WithHandle(entt::entity handle) const
    {
        return Entity(handle, m_Scene);
    }

    [[nodiscard]] explicit operator bool() const
    {
        return IsValid();
    }

    [[nodiscard]] bool operator==(const Entity&) const = default;

private:
    entt::entity m_Handle = entt::null;
    Scene* m_Scene = nullptr;
};
}  // namespace Matcha
