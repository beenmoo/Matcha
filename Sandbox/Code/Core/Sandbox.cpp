#include "Sandbox.h"
#include "CameraController.h"
#include "RotationComponent.h"
#include "Primitives.h"

#include <Matcha.h>

Sandbox::Sandbox(const Application::ApplicationSpecification& spec)
    : Application(spec)
{
    ResourceManager& resourceManager = GetContext().GetResourceManager();
    Scene& scene = GetContext().GetScene();
    Window& window = GetContext().GetWindow();

    ShaderHandle shader = resourceManager.CreateShader(
        "StandardMesh",
        {"Assets/Shaders/StandardMesh.vert", "Assets/Shaders/StandardMesh.frag"});

    CubePrimitive cubePrimitive;

    MeshHandle mesh = resourceManager.CreateMesh(
        cubePrimitive.vertices,
        {ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2},
        cubePrimitive.indices);

    m_Cube = scene.CreateEntity();
    m_Cube.AddComponent<TransformComponent>();
    m_Cube.AddComponent<MeshComponent>().mesh = mesh;
    m_Cube.AddComponent<NativeScriptComponent>().Bind<RotationComponent>();

    MaterialComponent& material = m_Cube.AddComponent<MaterialComponent>();
    material.shader = shader;
    material.albedoColor = Vector4(0.9f, 0.5f, 0.2f, 1.0f);

    Entity backpack = ModelLoader::LoadModel(scene, resourceManager, shader, "Assets/Models/survival_guitar_backpack/scene.gltf");

    if (backpack.IsValid())
    {
        // This particular export was authored in centimeters (a common Blender/Sketchfab glTF
        // export quirk) rather than glTF's spec-mandated meters, so its geometry is ~100x too
        // large/far from the origin at face value. Asset-specific, not something ModelLoader
        // should compensate for generally.
        Transform& transform = backpack.GetComponent<TransformComponent>().transform;
        transform.SetScale(0.005f, 0.005f, 0.005f);
        transform.SetPosition(1.5f, 0.0f, 0.0f);
    }

    Entity camera = scene.CreateEntity();
    camera.AddComponent<TransformComponent>().transform.SetPosition(0.0f, 0.0f, 3.0f);
    camera.AddComponent<CameraComponent>().aspectRatio = window.GetAspectRatio();
    camera.AddComponent<NativeScriptComponent>().Bind<CameraController>();

    // Points down and off to one side - rotating this entity is what aims the light, same as
    // rotating the camera entity aims the camera (LightSystem reads Transform::GetForward()).
    Entity light = scene.CreateEntity();
    light.AddComponent<TransformComponent>().transform.SetRotation(
        AngleAxis(Radians(-30.0f), Vector3(0.0f, 1.0f, 0.0f)) * AngleAxis(Radians(-50.0f), Vector3(1.0f, 0.0f, 0.0f)));
    auto& lightComp = light.AddComponent<LightComponent>();
    lightComp.intensity = 2.0f;

    // The spot light itself now lives on CameraController - it creates and drives its own
    // flashlight entity (see CameraController::OnCreate/OnUpdate) so it always follows the camera.
}

void Sandbox::OnUpdate()
{
}