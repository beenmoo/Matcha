#include "Sandbox.h"
#include "CameraController.h"
#include "RotationComponent.h"

#include <Matcha.h>

namespace Matcha
{
Sandbox::Sandbox(const Application::ApplicationSpecification& spec)
    : Application(spec)
{
    ResourceManager& resourceManager = GetContext().GetResourceManager();
    Scene& scene = GetContext().GetScene();
    Window& window = GetContext().GetWindow();

    ShaderHandle shader = resourceManager.CreateShader(
        "StandardMesh",
        {"Assets/Shaders/StandardMesh.vert", "Assets/Shaders/StandardMesh.frag"});

    // Each vertex: position (3), normal (3), texcoord (2). 4 vertices per face, rather than 8
    // shared corners, so each face gets its own flat normal instead of an averaged one.
    const float vertices[] = {
        // Front (+Z)
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,

        // Back (-Z)
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        1.0f,
        0.5f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,

        // Right (+X)
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,

        // Left (-X)
        -0.5f,
        -0.5f,
        -0.5f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        -0.5f,
        -0.5f,
        0.5f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        0.5f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        -0.5f,
        0.5f,
        -0.5f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,

        // Top (+Y)
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,

        // Bottom (-Y)
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
        1.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
    };

    uint32_t indices[36];

    for (uint32_t face = 0; face < 6; ++face)
    {
        uint32_t base = face * 4;
        uint32_t offset = face * 6;

        indices[offset + 0] = base + 0;
        indices[offset + 1] = base + 1;
        indices[offset + 2] = base + 2;
        indices[offset + 3] = base + 2;
        indices[offset + 4] = base + 3;
        indices[offset + 5] = base + 0;
    }

    MeshHandle mesh = resourceManager.CreateMesh(
        vertices,
        {ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2},
        indices);

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
}

void Sandbox::OnUpdate()
{
}
}  // namespace Matcha
