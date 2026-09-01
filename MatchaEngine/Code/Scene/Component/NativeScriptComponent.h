#pragma once

#include "Scene/ScriptableEntity.h"

#include <type_traits>
#include <vector>

namespace Matcha
{
struct NativeScriptComponent
{
    struct Binding
    {
        ScriptableEntity* instance = nullptr;

        ScriptableEntity* (*instantiateScript)() = nullptr;
        void (*destroyScript)(ScriptableEntity*) = nullptr;
    };

    // Multiple scripts can be bound to the same entity - each Bind<T>() call appends a new
    // Binding rather than replacing the previous one, so e.g. a camera entity can carry both
    // movement and flashlight-follow behavior as two independent scripts, neither aware of the
    // other, instead of one having to be written to know about the other.
    std::vector<Binding> bindings;

    NativeScriptComponent() = default;

    // Owns its instances (see ~NativeScriptComponent) - copying would leave two Bindings pointing
    // at the same instance, which then gets destroyed twice. Move stays available: entt's storage
    // moves components around during swap-and-pop erasure, and the default move leaves the
    // moved-from bindings vector empty, so its destructor safely does nothing.
    NativeScriptComponent(const NativeScriptComponent&) = delete;
    NativeScriptComponent& operator=(const NativeScriptComponent&) = delete;
    NativeScriptComponent(NativeScriptComponent&&) = default;
    NativeScriptComponent& operator=(NativeScriptComponent&&) = default;

    // Without this, entt::registry::destroy() (Scene::DestroyEntity, or the registry tearing down
    // on shutdown) frees the component's memory but never calls destroyScript - every instantiated
    // script instance was being leaked.
    ~NativeScriptComponent()
    {
        for (Binding& binding : bindings)
        {
            if (binding.instance)
                binding.destroyScript(binding.instance);
        }
    }

    template <typename T>
    void Bind()
    {
        static_assert(std::is_base_of_v<ScriptableEntity, T>, "Script must inherit from ScriptableEntity!");

        Binding binding;
        binding.instantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
        binding.destroyScript = [](ScriptableEntity* instance) { delete instance; };

        bindings.push_back(binding);
    }
};
}