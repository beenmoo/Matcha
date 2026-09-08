#pragma once

#include "Scene/Scene.h"

namespace Matcha
{
class Input;
class PythonRuntime;
class Time;

class PythonScriptSystem
{
public:
    PythonScriptSystem() = delete;

    static void Update(Scene& scene, Input& input, Time& time, PythonRuntime& pythonRuntime);
};
}  // namespace Matcha
