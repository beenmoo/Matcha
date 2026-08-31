#pragma once

namespace Matcha
{
class Scene;

class CameraSystem
{
public:
    CameraSystem() = delete;

    // Recomputes every CameraComponent::projection from its projection parameters.
    static void Update(Scene& scene);
};
}  // namespace Matcha
