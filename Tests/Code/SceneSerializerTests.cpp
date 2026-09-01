#include "pch.h"
#include "Graphics/ResourceManager.h"
#include "Scene/Component/CameraComponent.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/LightComponent.h"
#include "Scene/Component/MaterialComponent.h"
#include "Scene/Component/MeshComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"
#include "Scene/SceneSerializer.h"

#include <entt/entt.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

using namespace Matcha;

namespace
{
// A single scratch file, unique per test and removed on teardown - mirrors FileWatcherTests.cpp's
// TempDirectory, just for one file rather than a directory.
class TempFile
{
public:
    TempFile()
    {
        auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        m_Path = std::filesystem::temp_directory_path() / ("MatchaSceneSerializerTest-" + std::to_string(unique) + ".json");
    }

    ~TempFile()
    {
        std::error_code ec;
        std::filesystem::remove(m_Path, ec);
    }

    [[nodiscard]] std::string GetPath() const
    {
        return m_Path.string();
    }

private:
    std::filesystem::path m_Path;
};
}  // namespace

TEST(SceneSerializerTests, RoundTripsTagAndTransform)
{
    TempFile file;
    ResourceManager resourceManager;

    Scene sourceScene;
    Entity entity = sourceScene.CreateEntity("Player");
    entity.GetComponent<TagComponent>().isActive = false;
    entity.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 2.0f, 3.0f);
    entity.GetComponent<TransformComponent>().transform.SetScale(4.0f, 5.0f, 6.0f);
    entity.GetComponent<TransformComponent>().transform.SetRotationEuler(0.0f, 90.0f, 0.0f);

    SceneSerializer::Serialize(file.GetPath(), &sourceScene, resourceManager);

    Scene loadedScene;
    SceneSerializer::Deserialize(file.GetPath(), &loadedScene, resourceManager);

    std::vector<Entity> roots = loadedScene.GetRootEntities();
    ASSERT_EQ(roots.size(), 1u);

    const TagComponent& tag = roots[0].GetComponent<TagComponent>();
    EXPECT_EQ(tag.name, "Player");
    EXPECT_EQ(tag.id, entity.GetComponent<TagComponent>().id);
    EXPECT_FALSE(tag.isActive);

    const Transform& transform = roots[0].GetComponent<TransformComponent>().transform;
    EXPECT_EQ(transform.GetPosition(), Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(transform.GetScale(), Vector3(4.0f, 5.0f, 6.0f));

    Vector3 eulers = transform.GetRotationEuler();
    EXPECT_NEAR(eulers.y, Radians(90.0f), 0.001f);
}

TEST(SceneSerializerTests, RoundTripsLightComponent)
{
    TempFile file;
    ResourceManager resourceManager;

    Scene sourceScene;
    Entity entity = sourceScene.CreateEntity("Sun");
    LightComponent& light = entity.AddComponent<LightComponent>();
    light.type = LightType::Spot;
    light.color = Vector3(0.5f, 0.6f, 0.7f);
    light.intensity = 2.5f;
    light.range = 15.0f;
    light.innerConeAngle = 10.0f;
    light.outerConeAngle = 25.0f;
    light.ambientStrength = 0.2f;
    light.ambientColor = Vector3(0.1f, 0.1f, 0.1f);
    light.castShadows = true;

    SceneSerializer::Serialize(file.GetPath(), &sourceScene, resourceManager);

    Scene loadedScene;
    SceneSerializer::Deserialize(file.GetPath(), &loadedScene, resourceManager);

    std::vector<Entity> roots = loadedScene.GetRootEntities();
    ASSERT_EQ(roots.size(), 1u);
    ASSERT_TRUE(roots[0].HasComponent<LightComponent>());

    const LightComponent& loaded = roots[0].GetComponent<LightComponent>();
    EXPECT_EQ(loaded.type, LightType::Spot);
    EXPECT_EQ(loaded.color, Vector3(0.5f, 0.6f, 0.7f));
    EXPECT_FLOAT_EQ(loaded.intensity, 2.5f);
    EXPECT_FLOAT_EQ(loaded.range, 15.0f);
    EXPECT_FLOAT_EQ(loaded.innerConeAngle, 10.0f);
    EXPECT_FLOAT_EQ(loaded.outerConeAngle, 25.0f);
    EXPECT_FLOAT_EQ(loaded.ambientStrength, 0.2f);
    EXPECT_EQ(loaded.ambientColor, Vector3(0.1f, 0.1f, 0.1f));
    EXPECT_TRUE(loaded.castShadows);
}

TEST(SceneSerializerTests, RoundTripsCameraComponent)
{
    TempFile file;
    ResourceManager resourceManager;

    Scene sourceScene;
    Entity entity = sourceScene.CreateEntity("MainCamera");
    CameraComponent& camera = entity.AddComponent<CameraComponent>();
    camera.projectionType = CameraProjectionType::Orthographic;
    camera.orthographicSize = 20.0f;
    camera.orthographicNear = -5.0f;
    camera.orthographicFar = 5.0f;
    camera.aspectRatio = 1.777f;
    camera.primary = false;
    camera.fixedAspectRatio = true;

    SceneSerializer::Serialize(file.GetPath(), &sourceScene, resourceManager);

    Scene loadedScene;
    SceneSerializer::Deserialize(file.GetPath(), &loadedScene, resourceManager);

    std::vector<Entity> roots = loadedScene.GetRootEntities();
    ASSERT_EQ(roots.size(), 1u);
    ASSERT_TRUE(roots[0].HasComponent<CameraComponent>());

    const CameraComponent& loaded = roots[0].GetComponent<CameraComponent>();
    EXPECT_EQ(loaded.projectionType, CameraProjectionType::Orthographic);
    EXPECT_FLOAT_EQ(loaded.orthographicSize, 20.0f);
    EXPECT_FLOAT_EQ(loaded.orthographicNear, -5.0f);
    EXPECT_FLOAT_EQ(loaded.orthographicFar, 5.0f);
    EXPECT_FLOAT_EQ(loaded.aspectRatio, 1.777f);
    EXPECT_FALSE(loaded.primary);
    EXPECT_TRUE(loaded.fixedAspectRatio);
}

TEST(SceneSerializerTests, RoundTripsHierarchy)
{
    TempFile file;
    ResourceManager resourceManager;

    Scene sourceScene;
    Entity parent = sourceScene.CreateEntity("Parent");
    Entity childA = sourceScene.CreateEntity("ChildA");
    Entity childB = sourceScene.CreateEntity("ChildB");
    SetParent(childA, parent);
    SetParent(childB, parent);

    SceneSerializer::Serialize(file.GetPath(), &sourceScene, resourceManager);

    Scene loadedScene;
    SceneSerializer::Deserialize(file.GetPath(), &loadedScene, resourceManager);

    std::vector<Entity> roots = loadedScene.GetRootEntities();
    ASSERT_EQ(roots.size(), 1u);
    EXPECT_EQ(roots[0].GetComponent<TagComponent>().name, "Parent");
    ASSERT_TRUE(roots[0].HasComponent<HierarchyComponent>());
    EXPECT_EQ(roots[0].GetComponent<HierarchyComponent>().childrenCount, 2u);

    std::vector<std::string> childNames;
    entt::entity childHandle = roots[0].GetComponent<HierarchyComponent>().firstChild;
    while (childHandle != entt::null)
    {
        Entity child = roots[0].WithHandle(childHandle);
        childNames.push_back(child.GetComponent<TagComponent>().name);
        childHandle = child.GetComponent<HierarchyComponent>().nextSibling;
    }

    EXPECT_EQ(childNames.size(), 2u);
    EXPECT_NE(std::find(childNames.begin(), childNames.end(), "ChildA"), childNames.end());
    EXPECT_NE(std::find(childNames.begin(), childNames.end(), "ChildB"), childNames.end());
}

TEST(SceneSerializerTests, DeserializeOfMissingFileLeavesSceneEmpty)
{
    ResourceManager resourceManager;
    Scene scene;
    SceneSerializer::Deserialize("this/path/does/not/exist.json", &scene, resourceManager);

    EXPECT_TRUE(scene.GetRootEntities().empty());
}

// A MeshHandle ResourceManager never actually created (e.g. one that came from a call to the
// real ResourceManager::CreateMesh, which issues GL calls this test binary has no context for -
// see RendererTests.cpp) has no registered primitive kind, the same as an imported model's mesh.
// This exercises that "not a regeneratable primitive" skip path without needing a live GL context:
// constructing a MeshHandle directly bypasses ResourceManager entirely, so no GL call happens.
TEST(SceneSerializerTests, MeshWithUnknownHandleIsNotSerialized)
{
    TempFile file;
    ResourceManager resourceManager;

    Scene sourceScene;
    Entity entity = sourceScene.CreateEntity("UnknownMesh");
    entity.AddComponent<MeshComponent>().mesh = MeshHandle(12345);

    SceneSerializer::Serialize(file.GetPath(), &sourceScene, resourceManager);

    Scene loadedScene;
    SceneSerializer::Deserialize(file.GetPath(), &loadedScene, resourceManager);

    std::vector<Entity> roots = loadedScene.GetRootEntities();
    ASSERT_EQ(roots.size(), 1u);
    EXPECT_FALSE(roots[0].HasComponent<MeshComponent>());
}

// Shader/texture handles need a live GL context to create for real (same constraint as
// MeshWithUnknownHandleIsNotSerialized above), so this only covers MaterialComponent's plain
// fields with both handles left invalid (the component's own default) - confirms those round-trip
// and that no shaderPaths/texturePath key gets written for a handle nothing was ever created for.
TEST(SceneSerializerTests, RoundTripsMaterialPlainFieldsWithoutHandles)
{
    TempFile file;
    ResourceManager resourceManager;

    Scene sourceScene;
    Entity entity = sourceScene.CreateEntity("UnlitQuad");
    MaterialComponent& material = entity.AddComponent<MaterialComponent>();
    material.albedoColor = Vector4(0.2f, 0.4f, 0.6f, 1.0f);
    material.specularStrength = 0.75f;
    material.shininess = 16.0f;

    SceneSerializer::Serialize(file.GetPath(), &sourceScene, resourceManager);

    Scene loadedScene;
    SceneSerializer::Deserialize(file.GetPath(), &loadedScene, resourceManager);

    std::vector<Entity> roots = loadedScene.GetRootEntities();
    ASSERT_EQ(roots.size(), 1u);
    ASSERT_TRUE(roots[0].HasComponent<MaterialComponent>());

    const MaterialComponent& loaded = roots[0].GetComponent<MaterialComponent>();
    EXPECT_EQ(loaded.albedoColor, Vector4(0.2f, 0.4f, 0.6f, 1.0f));
    EXPECT_FLOAT_EQ(loaded.specularStrength, 0.75f);
    EXPECT_FLOAT_EQ(loaded.shininess, 16.0f);
    EXPECT_FALSE(loaded.shader.IsValid());
    EXPECT_FALSE(loaded.texture.IsValid());
}
