#pragma once

namespace Matcha
{
class Scene;
class Renderer;

class LightSystem
{
public:
    LightSystem() = delete;

    static void Update(Scene& scene, Renderer& renderer);
};
}  // namespace Matcha
