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

    // Submits every MeshComponent/MaterialComponent entity to renderer as-is, without touching
    // view-projection - the caller (Update() above, or a non-scene camera source like
    // MatchaEditor's EditorCamera) is responsible for calling Renderer::SetViewProjection() first.
    static void Draw(Scene& scene, Renderer& renderer);
};
}  // namespace Matcha
