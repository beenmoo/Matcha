#pragma once

#include "RenderHandles.h"
#include "RendererAPI.h"
#include "ResourceManager.h"
#include "UniformBuffer.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace Matcha
{
struct RenderData
{
    MeshHandle mesh;
    ShaderHandle shader;
    TextureHandle texture;

    // World-space transform. View-projection is uploaded separately, once per frame, via
    // SetViewProjection - it is not baked in here.
    Matrix4 transform;

    Vector4 albedoColor = Vector4(1.0f);
    float specularStrength = 0.5f;
    float shininess = 32.0f;
};

// Mirrors StandardMesh.frag's LightBlock.Light struct field-for-field. This only works as a
// direct memcpy-able C++ struct because every vec3 here is immediately followed by nothing but a
// scalar, which both std140 and plain C++ pack into the same 16-byte slot with no gap - a layout
// with a vec3 following an int/scalar-pair (like LightBlock's own header) would need explicit
// padding fields instead, since only std140 forces that gap.
struct LightData
{
    Vector3 position;
    float type = 0.0f;  // LightType's ordinal: 0 = Directional, 1 = Point, 2 = Spot

    Vector3 color = Vector3(1.0f);
    float intensity = 1.0f;

    Vector3 direction = Vector3(0.0f, -1.0f, 0.0f);
    float range = 10.0f;

    // Cosines of the spot cone's inner/outer half-angles, not angles - compared directly against
    // a dot product in the shader.
    float innerCutOff = 1.0f;
    float outerCutOff = 1.0f;
    Vector2 padding;
};

inline constexpr int MAX_LIGHTS = 16;

class Renderer
{
public:
    Renderer(RendererAPI& rendererAPI, ResourceManager& resourceManager);

    // Creates GL-backed resources (the camera uniform buffer). Deferred out of the constructor
    // and called from Application::InitGraphics() instead, alongside RendererAPI::Init(): under
    // the Qt backend, the GL context isn't current and glad isn't loaded yet at the point
    // Application's constructor init list runs (SDL's is, which is why this worked as
    // constructor-time code before Qt existed).
    void Init();

    void Submit(const RenderData& renderData);
    void Flush();
    void Clear();
    void SetClearColor(const Vector4& color);

    // Uploads the given view-projection to the CameraBlock uniform buffer (binding 0), shared by
    // every shader that declares it. Intended to be called once per frame, before Flush().
    void SetViewProjection(const Matrix4& viewProjection);

    // Uploads the camera's world-space position into the same CameraBlock, past the
    // view-projection matrix. Needed by shaders computing a view direction (e.g. specular).
    void SetCameraPosition(const Vector3& position);

    // Uploads up to MAX_LIGHTS entries to the LightBlock uniform buffer's light array (binding 1),
    // plus the light count. Extra entries past MAX_LIGHTS are silently dropped.
    void SetLights(const std::vector<LightData>& lights);

    // Flat scene-wide fill light, uploaded to the same LightBlock's header. Not physically tied to
    // any specific light source.
    void SetAmbient(float strength, const Vector3& color);

private:
    void SortRenderData();

private:
    RendererAPI& m_RendererAPI;
    ResourceManager& m_ResourceManager;

    std::unique_ptr<UniformBuffer> m_CameraUniformBuffer;
    std::unique_ptr<UniformBuffer> m_LightUniformBuffer;

    // Bound in place of RenderData::texture when it's invalid (no albedo map), so the shader can
    // just always sample u_AlbedoMap unconditionally instead of branching on a u_HasAlbedoMap
    // uniform. A single opaque white pixel is the identity for albedo * texture.
    TextureHandle m_DefaultWhiteTexture;

    std::vector<RenderData> m_RenderData;
};
}  // namespace Matcha
