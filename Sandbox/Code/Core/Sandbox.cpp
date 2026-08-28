#include "Sandbox.h"

#include <Matcha.h>

#include <algorithm>

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

     Entity backpack = ModelLoader::LoadModel(scene, resourceManager, shader, "Assets/Models/survival_guitar_backpack/scene.gltf");

     if (backpack.IsValid())
     {
         // This particular export was authored in centimeters (a common Blender/Sketchfab glTF
         // export quirk) rather than glTF's spec-mandated meters, so its geometry is ~100x too
         // large/far from the origin at face value. Asset-specific, not something ModelLoader
         // should compensate for generally.
         Transform& transform = backpack.GetComponent<TransformComponent>().transform;
         transform.SetScale(0.001f, 0.001f, 0.001f);
         transform.SetPosition(1.5f, 0.0f, 0.0f);
     }

    m_Camera = scene.CreateEntity();
    m_Camera.AddComponent<TransformComponent>().transform.SetPosition(0.0f, 0.0f, 3.0f);
    m_Camera.AddComponent<CameraComponent>().aspectRatio = window.GetAspectRatio();
}

void Sandbox::OnUpdate()
{
    constexpr float degreesPerSecond = 45.0f;

    float deltaTime = GetContext().GetTime().GetDeltaTime();

    m_Cube.GetComponent<TransformComponent>().transform.Rotate(Vector3(0.0f, 1.0f, 0.0f), degreesPerSecond * deltaTime);

    Input& input = GetContext().GetInput();
    Transform& cameraTransform = m_Camera.GetComponent<TransformComponent>().transform;

    // Look: only while holding the right mouse button. Lock/hide the cursor for the duration so
    // it can't run out of window to move in - SetCursorLockState hides it and, on Qt, warps it
    // back to center each move (SDL's relative mouse mode does the equivalent natively).
    if (input.GetMouseButtonDown(Input::MouseButton::Right))
        input.SetCursorLockState(Input::CursorLockState::Locked);
    else if (input.GetMouseButtonUp(Input::MouseButton::Right))
        input.SetCursorLockState(Input::CursorLockState::None);

    if (input.GetMouseButton(Input::MouseButton::Right))
    {
        constexpr float mouseSensitivity = 0.15f;

        Vector2Int mouseDelta = input.GetAxis(Input::AxisType::Mouse);

        m_Yaw += -static_cast<float>(mouseDelta.x) * mouseSensitivity;
        m_Pitch += -static_cast<float>(mouseDelta.y) * mouseSensitivity;
        m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

        // Rebuilt fresh from the total yaw/pitch every time, not composed incrementally - yaw
        // around world up, pitch around the yawed frame's local right, matching the classic
        // FPS-camera R = Yaw * Pitch order (see the m_Yaw/m_Pitch comment in Sandbox.h for why).
        cameraTransform.SetRotation(AngleAxis(Radians(m_Yaw), Vector3(0.0f, 1.0f, 0.0f)) * AngleAxis(Radians(m_Pitch), Vector3(1.0f, 0.0f, 0.0f)));
    }

    // Move: WASD + Space/Ctrl, relative to the camera's own orientation.
    constexpr float moveSpeed = 2.0f;

    Vector3 movement(0.0f);

    if (input.GetKey(KeyCode::W))
        movement += cameraTransform.GetForward();
    if (input.GetKey(KeyCode::S))
        movement -= cameraTransform.GetForward();
    if (input.GetKey(KeyCode::D))
        movement += cameraTransform.GetRight();
    if (input.GetKey(KeyCode::A))
        movement -= cameraTransform.GetRight();
    if (input.GetKey(KeyCode::SPACE))
        movement += Vector3(0.0f, 1.0f, 0.0f);
    if (input.GetKey(KeyCode::LCTRL))
        movement -= Vector3(0.0f, 1.0f, 0.0f);

    if (movement != Vector3(0.0f))
        cameraTransform.Translate(Normalize(movement) * moveSpeed * deltaTime);
}
}  // namespace Matcha
