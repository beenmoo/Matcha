#include "RotationComponent.h"

#include <algorithm>

void RotationComponent::OnUpdate()
{
    float deltaTime = GetContext().GetTime().GetDeltaTime();

    Transform& transform = GetComponent<TransformComponent>().transform;
    transform.Rotate(Vector3(0.0f, 1.0f, 0.0f), degreesPerSecond * deltaTime);
}
