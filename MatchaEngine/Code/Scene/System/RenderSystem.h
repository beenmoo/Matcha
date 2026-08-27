#pragma once

#include "Scene/Scene.h"

namespace Matcha
{
class RenderSystem
{
public:
    RenderSystem() = delete;

    static void Update(Scene& scene);

private:
    static void Draw(Scene& scene);
};
}  // namespace Matcha
