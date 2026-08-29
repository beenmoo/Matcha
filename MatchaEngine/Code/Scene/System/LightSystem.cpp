#include "LightSystem.h"
#include "Graphics/Renderer.h"
#include "Scene/Component/LightComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"

#include <cmath>

namespace Matcha
{
void LightSystem::Update(Scene& scene, Renderer& renderer)
{
    auto view = scene.View<LightComponent, TransformComponent>();

    std::vector<LightData> lights;
    lights.reserve(MAX_LIGHTS);

    bool hasSetAmbient = false;

    for (auto&& [handle, light, transform] : view.each())
    {
        if (static_cast<int>(lights.size()) >= MAX_LIGHTS)
            break;

        LightData data;
        data.position = transform.transform.GetPosition();
        data.type = static_cast<float>(light.type);
        data.color = light.color;
        data.intensity = light.intensity;
        data.direction = transform.transform.GetForward();
        data.range = light.range;
        data.innerCutOff = std::cos(Radians(light.innerConeAngle));
        data.outerCutOff = std::cos(Radians(light.outerConeAngle));

        lights.push_back(data);

        // Ambient is scene-wide, not per-light - it just has nowhere else to be authored from yet,
        // so it rides along on LightComponent. Take it from whichever light entity is first.
        if (!hasSetAmbient)
        {
            renderer.SetAmbient(light.ambientStrength, light.ambientColor);
            hasSetAmbient = true;
        }
    }

    renderer.SetLights(lights);
}
}  // namespace Matcha
