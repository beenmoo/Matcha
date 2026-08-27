#pragma once

#include "Entity.h"

namespace Matcha
{
class ScriptableEntity
{
public:
    virtual ~ScriptableEntity() = default;

    template <typename T>
    [[nodiscard]] T& GetComponent()
    {
        return Entity::GetComponent<T>();
    }

    template<typename T>
    const T& GetComponent() const
    {
        return m_Entity.GetComponent<T>();
    }

    template <typename T>
    [[nodiscard]] bool HasComponent() const
    {
        return Entity::HasComponent<T>();
    }

protected:
    void Update() {};

private:
    friend class ScriptSystem;    

    Entity m_Entity;
};
}