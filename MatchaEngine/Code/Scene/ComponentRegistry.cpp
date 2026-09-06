#include "ComponentRegistry.h"
// Scene.h, not just Entity.h: Entity's templated component accessors are defined there (they need
// Scene to be a complete type), so without it every GetComponent/AddComponent below is declared
// but never instantiated.
#include "Scene.h"
#include "Component/CameraComponent.h"
#include "Component/MaterialComponent.h"
#include "Component/MeshComponent.h"
#include "Component/LightComponent.h"
#include "Component/PythonScriptComponent.h"
#include "Component/TransformComponent.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector.h"

namespace Matcha
{
namespace
{
nlohmann::json ToJson(const Vector3& v)
{
    return {v.x, v.y, v.z};
}

Vector3 Vector3FromJson(const nlohmann::json& j)
{
    return Vector3(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>());
}

nlohmann::json ToJson(const Quaternion& q)
{
    return {q.x, q.y, q.z, q.w};
}

Quaternion QuaternionFromJson(const nlohmann::json& j)
{
    return Quaternion(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>(), j.at(3).get<float>());
}

nlohmann::json ToJson(const Vector4& v)
{
    return {v.x, v.y, v.z, v.w};
}

Vector4 Vector4FromJson(const nlohmann::json& j)
{
    return Vector4(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>(), j.at(3).get<float>());
}
}  // namespace

const std::vector<ComponentSerializer>& GetComponentSerializers()
{
    static const std::vector<ComponentSerializer> serializers = {
        {"transform",
         [](nlohmann::json& entityNode, Entity entity, ResourceManager&) {
             if (!entity.HasComponent<TransformComponent>())
                 return;

             const Transform& transform = entity.GetComponent<TransformComponent>().transform;
             entityNode["transform"] = {
                 {"position", ToJson(transform.GetPosition())},
                 {"rotation", ToJson(transform.GetRotation())},
                 {"scale", ToJson(transform.GetScale())},
             };
         },
         [](const nlohmann::json& node, Entity entity, ResourceManager&) {
             // Already present: Scene::CreateEntity adds TransformComponent to every entity.
             Transform& transform = entity.GetComponent<TransformComponent>().transform;
             transform.SetPosition(Vector3FromJson(node.at("position")));
             transform.SetRotation(QuaternionFromJson(node.at("rotation")));
             transform.SetScale(Vector3FromJson(node.at("scale")));
         }},

        {"light",
         [](nlohmann::json& entityNode, Entity entity, ResourceManager&) {
             if (!entity.HasComponent<LightComponent>())
                 return;

             const LightComponent& light = entity.GetComponent<LightComponent>();
             entityNode["light"] = {
                 {"type", static_cast<int>(light.type)},
                 {"color", ToJson(light.color)},
                 {"intensity", light.intensity},
                 {"range", light.range},
                 {"innerConeAngle", light.innerConeAngle},
                 {"outerConeAngle", light.outerConeAngle},
                 {"ambientStrength", light.ambientStrength},
                 {"ambientColor", ToJson(light.ambientColor)},
                 {"castShadows", light.castShadows},
             };
         },
         [](const nlohmann::json& node, Entity entity, ResourceManager&) {
             LightComponent& light = entity.AddComponent<LightComponent>();
             light.type = static_cast<LightType>(node.at("type").get<int>());
             light.color = Vector3FromJson(node.at("color"));
             light.intensity = node.at("intensity").get<float>();
             light.range = node.at("range").get<float>();
             light.innerConeAngle = node.at("innerConeAngle").get<float>();
             light.outerConeAngle = node.at("outerConeAngle").get<float>();
             light.ambientStrength = node.at("ambientStrength").get<float>();
             light.ambientColor = Vector3FromJson(node.at("ambientColor"));
             light.castShadows = node.at("castShadows").get<bool>();
         }},

        {"camera",
         [](nlohmann::json& entityNode, Entity entity, ResourceManager&) {
             if (!entity.HasComponent<CameraComponent>())
                 return;

             const CameraComponent& camera = entity.GetComponent<CameraComponent>();
             entityNode["camera"] = {
                 {"projectionType", static_cast<int>(camera.projectionType)},
                 {"perspectiveFOV", camera.perspectiveFOV},
                 {"perspectiveNear", camera.perspectiveNear},
                 {"perspectiveFar", camera.perspectiveFar},
                 {"orthographicSize", camera.orthographicSize},
                 {"orthographicNear", camera.orthographicNear},
                 {"orthographicFar", camera.orthographicFar},
                 {"aspectRatio", camera.aspectRatio},
                 {"primary", camera.primary},
                 {"fixedAspectRatio", camera.fixedAspectRatio},
             };
         },
         [](const nlohmann::json& node, Entity entity, ResourceManager&) {
             CameraComponent& camera = entity.AddComponent<CameraComponent>();
             camera.projectionType = static_cast<CameraProjectionType>(node.at("projectionType").get<int>());
             camera.perspectiveFOV = node.at("perspectiveFOV").get<float>();
             camera.perspectiveNear = node.at("perspectiveNear").get<float>();
             camera.perspectiveFar = node.at("perspectiveFar").get<float>();
             camera.orthographicSize = node.at("orthographicSize").get<float>();
             camera.orthographicNear = node.at("orthographicNear").get<float>();
             camera.orthographicFar = node.at("orthographicFar").get<float>();
             camera.aspectRatio = node.at("aspectRatio").get<float>();
             camera.primary = node.at("primary").get<bool>();
             camera.fixedAspectRatio = node.at("fixedAspectRatio").get<bool>();
         }},

        {"mesh",
         [](nlohmann::json& entityNode, Entity entity, ResourceManager& resourceManager) {
             if (!entity.HasComponent<MeshComponent>())
                 return;

             MeshHandle mesh = entity.GetComponent<MeshComponent>().mesh;
             if (!mesh.IsValid())
                 return;

             // An imported mesh has no primitiveKind and so nothing to regenerate it from - it's
             // skipped rather than written as a handle that means nothing after a reload.
             std::string primitiveKind = resourceManager.GetMeshPrimitiveKind(mesh);
             if (!primitiveKind.empty())
                 entityNode["mesh"] = {{"primitive", primitiveKind}};
         },
         [](const nlohmann::json& node, Entity entity, ResourceManager& resourceManager) {
             MeshHandle mesh = resourceManager.GetOrCreatePrimitiveMesh(node.at("primitive").get<std::string>());
             if (mesh.IsValid())
                 entity.AddComponent<MeshComponent>().mesh = mesh;
         }},

        {"material",
         [](nlohmann::json& entityNode, Entity entity, ResourceManager& resourceManager) {
             if (!entity.HasComponent<MaterialComponent>())
                 return;

             const MaterialComponent& material = entity.GetComponent<MaterialComponent>();

             nlohmann::json materialNode = {
                 {"albedoColor", ToJson(material.albedoColor)},
                 {"specularStrength", material.specularStrength},
                 {"shininess", material.shininess},
             };

             // shader/texture are handles, not data - only serializable via what the underlying
             // Shader/Texture object can report about its own origin (Shader::GetPaths()/
             // Texture::GetPath()). A procedural texture (CreateTexture(width, height)) has an
             // empty path and is skipped the same way an imported mesh is above.
             if (material.shader.IsValid())
             {
                 if (Shader* shader = resourceManager.GetShader(material.shader); shader && !shader->GetPaths().empty())
                     materialNode["shaderPaths"] = shader->GetPaths();
             }

             if (material.texture.IsValid())
             {
                 if (Texture* texture = resourceManager.GetTexture(material.texture); texture && !texture->GetPath().empty())
                     materialNode["texturePath"] = texture->GetPath();
             }

             entityNode["material"] = std::move(materialNode);
         },
         [](const nlohmann::json& node, Entity entity, ResourceManager& resourceManager) {
             MaterialComponent& material = entity.AddComponent<MaterialComponent>();
             material.albedoColor = Vector4FromJson(node.at("albedoColor"));
             material.specularStrength = node.at("specularStrength").get<float>();
             material.shininess = node.at("shininess").get<float>();

             if (node.contains("shaderPaths"))
             {
                 std::vector<std::string> shaderPaths = node.at("shaderPaths").get<std::vector<std::string>>();
                 if (!shaderPaths.empty())
                     material.shader = resourceManager.CreateShader("Material", shaderPaths);
             }

             if (node.contains("texturePath"))
                 material.texture = resourceManager.CreateTexture(node.at("texturePath").get<std::string>());
         }},

        {"pythonScript",
         [](nlohmann::json& entityNode, Entity entity, ResourceManager&) {
             if (!entity.HasComponent<PythonScriptComponent>())
                 return;

             const PythonScriptComponent& script = entity.GetComponent<PythonScriptComponent>();

             nlohmann::json bindingsNode = nlohmann::json::array();
             for (const PythonScriptComponent::Binding& binding : script.bindings)
             {
                 // An empty moduleName means this binding was added via the Inspector's
                 // "Browse..." flow but never actually resolved to anything - nothing meaningful
                 // to write, same reasoning as an imported mesh's unregeneratable handle above.
                 if (binding.moduleName.empty())
                     continue;

                 bindingsNode.push_back({{"module", binding.moduleName}, {"class", binding.className}});
             }

             if (!bindingsNode.empty())
                 entityNode["pythonScript"] = std::move(bindingsNode);
         },
         [](const nlohmann::json& node, Entity entity, ResourceManager&) {
             // (moduleName, className) is a name pair Python's own import system resolves, not a
             // handle - unlike NativeScriptComponent's old compiled function pointers, there's
             // nothing here that needs a registry to serialize as data. Deliberately doesn't
             // re-register a script directory: the entity that added this binding already went
             // through PythonRuntime::RegisterScriptDirectory once (Sandbox.cpp at startup, or the
             // Inspector's "Browse..." picker), and that registration is process-wide, not
             // per-scene state to restore.
             PythonScriptComponent& script = entity.AddComponent<PythonScriptComponent>();
             for (const nlohmann::json& bindingNode : node)
                 script.Bind(bindingNode.at("module").get<std::string>(), bindingNode.at("class").get<std::string>());
         }},
    };

    return serializers;
}
}  // namespace Matcha
