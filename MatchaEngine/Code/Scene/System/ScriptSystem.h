#pragma once

#include "Scene/Scene.h"

namespace Matcha
{
class ScriptSystem
{
public:
    ScriptSystem() = delete;

    static void Update(Scene& scene);
};
}