#include "pch.h"
#include "MathTestUtils.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"
#include "Scene/System/TransformSystem.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaTests;

namespace
{
Vector3 ExtractTranslation(const Matrix4& matrix)
{
    Vector3 scale;
    Quaternion rotation;
    Vector3 translation;
    Vector3 skew;
    Vector4 perspective;

    bool success = Decompose(matrix, scale, rotation, translation, skew, perspective);
    EXPECT_TRUE(success);

    return translation;
}
}  // namespace

TEST(TransformSystemTests, RootWithNoHierarchyWorldMatrixMatchesLocal)
{
    Scene scene;
    Entity entity = scene.CreateEntity();
    entity.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 2.0f, 3.0f);

    TransformSystem::Update(scene);

    ExpectMatrixNear(entity.GetComponent<TransformComponent>().worldMatrix, entity.GetComponent<TransformComponent>().transform.GetLocalMatrix());
}

TEST(TransformSystemTests, RootWithHierarchyWorldMatrixMatchesLocal)
{
    Scene scene;
    Entity root = scene.CreateEntity();
    root.GetComponent<TransformComponent>().transform.SetPosition(5.0f, 0.0f, 0.0f);
    root.AddComponent<HierarchyComponent>();

    TransformSystem::Update(scene);

    ExpectVectorNear(ExtractTranslation(root.GetComponent<TransformComponent>().worldMatrix), Vector3(5.0f, 0.0f, 0.0f));
}

TEST(TransformSystemTests, ChildWorldMatrixComposesParentTranslation)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    parent.GetComponent<TransformComponent>().transform.SetPosition(5.0f, 0.0f, 0.0f);
    child.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 0.0f, 0.0f);
    SetParent(child, parent);

    TransformSystem::Update(scene);

    ExpectVectorNear(ExtractTranslation(child.GetComponent<TransformComponent>().worldMatrix), Vector3(6.0f, 0.0f, 0.0f));
}

TEST(TransformSystemTests, GrandchildWorldMatrixComposesThreeLevels)
{
    Scene scene;
    Entity grandparent = scene.CreateEntity();
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    grandparent.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 0.0f, 0.0f);
    parent.GetComponent<TransformComponent>().transform.SetPosition(0.0f, 1.0f, 0.0f);
    child.GetComponent<TransformComponent>().transform.SetPosition(0.0f, 0.0f, 1.0f);

    SetParent(parent, grandparent);
    SetParent(child, parent);

    TransformSystem::Update(scene);

    ExpectVectorNear(ExtractTranslation(child.GetComponent<TransformComponent>().worldMatrix), Vector3(1.0f, 1.0f, 1.0f));
}

TEST(TransformSystemTests, ParentScalePropagatesToChildWorldPosition)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    parent.GetComponent<TransformComponent>().transform.SetScale(2.0f, 2.0f, 2.0f);
    child.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 0.0f, 0.0f);
    SetParent(child, parent);

    TransformSystem::Update(scene);

    ExpectVectorNear(ExtractTranslation(child.GetComponent<TransformComponent>().worldMatrix), Vector3(2.0f, 0.0f, 0.0f));
}

TEST(TransformSystemTests, UpdateIsIdempotentAndDoesNotMutateLocalTransform)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    parent.GetComponent<TransformComponent>().transform.SetPosition(5.0f, 0.0f, 0.0f);
    child.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 0.0f, 0.0f);
    SetParent(child, parent);

    TransformSystem::Update(scene);
    TransformSystem::Update(scene);

    ExpectVectorNear(child.GetComponent<TransformComponent>().transform.GetPosition(), Vector3(1.0f, 0.0f, 0.0f));
    ExpectVectorNear(ExtractTranslation(child.GetComponent<TransformComponent>().worldMatrix), Vector3(6.0f, 0.0f, 0.0f));
}
