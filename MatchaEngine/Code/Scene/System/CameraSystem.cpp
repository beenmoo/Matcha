#include "CameraSystem.h"

#include "Math/Matrix.h"
#include "Scene/Component/CameraComponent.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Scene.h"

namespace Matcha
{
void CameraSystem::Update(Scene& scene)
{
    auto view = scene.View<CameraComponent>();

    for (auto handle : view)
    {
        if (!IsActiveInHierarchy(Entity(handle, &scene)))
            continue;

        CameraComponent& camera = view.get<CameraComponent>(handle);

        if (camera.projectionType == CameraProjectionType::Perspective)
        {
            camera.projection = Perspective(camera.perspectiveFOV, camera.aspectRatio, camera.perspectiveNear, camera.perspectiveFar);
        }
        else
        {
            float orthoLeft = -camera.orthographicSize * camera.aspectRatio * 0.5f;
            float orthoRight = camera.orthographicSize * camera.aspectRatio * 0.5f;
            float orthoBottom = -camera.orthographicSize * 0.5f;
            float orthoTop = camera.orthographicSize * 0.5f;

            camera.projection = Orthographic(orthoLeft, orthoRight, orthoBottom, orthoTop, camera.orthographicNear, camera.orthographicFar);
        }
    }
}
}  // namespace Matcha
