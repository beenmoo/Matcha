#include "Flashlight.h"

void Flashlight::OnCreate()
{
    Scene& scene = GetContext().GetScene();

    m_Light = scene.CreateEntity();
    m_Light.AddComponent<TransformComponent>();

    LightComponent& light = m_Light.AddComponent<LightComponent>();
    light.type = LightType::Spot;
    light.color = Vector3(0.6f, 0.8f, 1.0f);
    light.intensity = 5.0f;
    light.range = 6.0f;
    light.innerConeAngle = 12.5f;
    light.outerConeAngle = 20.0f;
}

void Flashlight::OnUpdate()
{
    Transform& hostTransform = GetComponent<TransformComponent>().transform;
    Transform& lightTransform = m_Light.GetComponent<TransformComponent>().transform;

    lightTransform.SetPosition(hostTransform.GetPosition());
    lightTransform.SetRotation(hostTransform.GetRotation());
}
