#include "LightSystem.h"
#include "Graphics/Renderer.h"
#include "Scene/Component/LightComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"

#include <array>
#include <cmath>
#include <span>

namespace Matcha
{
void LightSystem::Update(Scene& scene, Renderer& renderer)
{
    auto view = scene.View<LightComponent, TransformComponent>();

    // Fixed-size and capped at MAX_LIGHTS rather than a std::vector - this runs every frame, and
    // the light count never exceeds what Renderer::SetLights caps at anyway, so there's no reason
    // to heap-allocate for it.
    std::array<LightData, MAX_LIGHTS> lights;
    size_t lightCount = 0;

    bool hasSetAmbient = false;

    for (auto&& [handle, light, transform] : view.each())
    {
        if (lightCount >= MAX_LIGHTS)
            break;

        LightData& data = lights[lightCount++];
        data.position = transform.transform.GetPosition();
        data.type = static_cast<float>(light.type);
        data.color = light.color;
        data.intensity = light.intensity;
        data.direction = transform.transform.GetForward();
        data.range = light.range;
        data.innerCutOff = std::cos(Radians(light.innerConeAngle));
        data.outerCutOff = std::cos(Radians(light.outerConeAngle));

        // Ambient is scene-wide, not per-light - it just has nowhere else to be authored from yet,
        // so it rides along on LightComponent. Take it from whichever light entity is first.
        if (!hasSetAmbient)
        {
            renderer.SetAmbient(light.ambientStrength, light.ambientColor);
            hasSetAmbient = true;
        }
    }

    renderer.SetLights(std::span(lights.data(), lightCount));
}
}  // namespace Matcha
