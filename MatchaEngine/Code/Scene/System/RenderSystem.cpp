#include "RenderSystem.h"

#include "Graphics/Renderer.h"
#include "Scene/Component/CameraComponent.h"
#include "Scene/Component/MaterialComponent.h"
#include "Scene/Component/MeshComponent.h"
#include "Scene/Component/TransformComponent.h"

namespace Matcha
{
void RenderSystem::Update(Scene& scene, Renderer& renderer)
{
    auto cameraView = scene.View<CameraComponent, TransformComponent>();

    for (auto&& [handle, camera, transform] : cameraView.each())
    {
        if (!camera.primary)
            continue;

        Matrix4 viewProjection = camera.projection * Inverse(transform.worldMatrix);
        renderer.SetViewProjection(viewProjection);

        Draw(scene, renderer);
        return;
    }
}

void RenderSystem::Draw(Scene& scene, Renderer& renderer)
{
    auto view = scene.View<MeshComponent, MaterialComponent, TransformComponent>();

    for (auto&& [handle, mesh, material, transform] : view.each())
    {
        RenderData renderData;
        renderData.mesh = mesh.mesh;
        renderData.shader = material.shader;
        renderData.texture = material.texture;
        renderData.transform = transform.worldMatrix;
        renderData.albedoColor = material.albedoColor;

        renderer.Submit(renderData);
    }
}
}  // namespace Matcha
