#pragma once

#include <type_traits>

namespace Matcha
{
class ScriptableEntity;

struct NativeScriptComponent
{
    ScriptableEntity* instance = nullptr;

    ScriptableEntity* (*instantiateScript)();
    void (*destroyScript)(NativeScriptComponent*);

    template <typename T>
    void Bind()
    {
        instantiateScript = []() { static_assert(std::is_base_of_v<ScriptableEntity, T>, "Script must inherit from ScriptableEntity!"); return static_cast<ScriptableEntity*>(new T()); };
        destroyScript = [](NativeScriptComponent* nsc) { delete nsc->instance; nsc->instance = nullptr; };
    }
};
}