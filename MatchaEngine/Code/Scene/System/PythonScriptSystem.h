#pragma once

#include "Scene/Scene.h"

namespace Matcha
{
class EngineContext;

class PythonScriptSystem
{
public:
    PythonScriptSystem() = delete;

    static void Update(Scene& scene, EngineContext& context);
};
}  // namespace Matcha
