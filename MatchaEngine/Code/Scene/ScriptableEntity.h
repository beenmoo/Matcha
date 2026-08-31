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

    // Called once, right after the script's Entity/EngineContext are set (before its first
    // OnUpdate()) - for setup that needs those (e.g. creating other entities), which isn't safe to
    // do from the constructor since neither is available yet at that point.
    virtual void OnCreate()
    {
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