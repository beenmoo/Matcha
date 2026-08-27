#include "Sandbox.h"

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
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,

        // Back (-Z)
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,

        // Right (+X)
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // Left (-X)
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // Top (+Y)
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        // Bottom (-Y)
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
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

    MaterialComponent& material = m_Cube.AddComponent<MaterialComponent>();
    material.shader = shader;
    material.albedoColor = Vector4(0.9f, 0.5f, 0.2f, 1.0f);

    Entity camera = scene.CreateEntity();
    camera.AddComponent<TransformComponent>().transform.SetPosition(0.0f, 0.0f, 3.0f);
    camera.AddComponent<CameraComponent>().aspectRatio = window.GetAspectRatio();
}

void Sandbox::OnUpdate()
{
    constexpr float degreesPerSecond = 45.0f;

    float deltaTime = GetContext().GetTime().GetDeltaTime();

    m_Cube.GetComponent<TransformComponent>().transform.Rotate(Vector3(0.0f, 1.0f, 0.0f), degreesPerSecond * deltaTime);
}
}  // namespace Matcha
