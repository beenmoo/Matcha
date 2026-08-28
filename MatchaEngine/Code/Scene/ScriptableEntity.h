#pragma once

#include "Entity.h"

namespace Matcha
{
class EngineContext;

class ScriptableEntity
{
public:
    virtual ~ScriptableEntity() = default;

    template <typename T>
    [[nodiscard]] T& GetComponent()
    {
        return m_Entity.GetComponent<T>();
    }

    template <typename T>
    [[nodiscard]] const T& GetComponent() const
    {
        return m_Entity.GetComponent<T>();
    }

    template <typename T>
    [[nodiscard]] bool HasComponent() const
    {
        return m_Entity.HasComponent<T>();
    }

protected:
    // Application's Input/Time/etc, the same bundle Application::OnUpdate() itself gets via
    // GetContext() - scripts need it for exactly the same reasons (reading input, delta time via
    // GetContext().GetTime().GetDeltaTime(), matching how Application::OnUpdate() itself reads it
    // rather than receiving it as a parameter).
    [[nodiscard]] EngineContext& GetContext() const
    {
        return *m_Context;
    }

    virtual void OnUpdate()
    {
    }

private:
    friend class ScriptSystem;

    Entity m_Entity;
    EngineContext* m_Context = nullptr;
};
}  // namespace Matcha