#pragma once

#include "Math/Matrix.h"
#include "Scene/Scene.h"

namespace Matcha
{
class Renderer;

class RenderSystem
{
public:
    RenderSystem() = delete;

    // Finds the primary camera and submits every MeshComponent/MaterialComponent entity to
    // renderer, transformed by that camera's view-projection.
    static void Update(Scene& scene, Renderer& renderer);

private:
    static void Draw(Scene& scene, Renderer& renderer);
};
}  // namespace Matcha
