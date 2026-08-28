#pragma once

#include "Scene/Scene.h"

namespace Matcha
{
class EngineContext;

class ScriptSystem
{
public:
    ScriptSystem() = delete;

    static void Update(Scene& scene, EngineContext& context);
};
}  // namespace Matcha
