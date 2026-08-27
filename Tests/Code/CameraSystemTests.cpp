#include "pch.h"
#include "MathTestUtils.h"
#include "Scene/Component/CameraComponent.h"
#include "Scene/Scene.h"
#include "Scene/System/CameraSystem.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaTests;

TEST(CameraSystemTests, PerspectiveProjectionMatchesDirectCall)
{
    Scene scene;
    Entity entity = scene.CreateEntity();

    CameraComponent& camera = entity.AddComponent<CameraComponent>();
    camera.projectionType = CameraProjectionType::Perspective;
    camera.perspectiveFOV = 60.0f;
    camera.perspectiveNear = 0.3f;
    camera.perspectiveFar = 500.0f;
    camera.aspectRatio = 1.5f;

    CameraSystem::Update(scene);

    ExpectMatrixNear(camera.projection, Perspective(60.0f, 1.5f, 0.3f, 500.0f));
}

TEST(CameraSystemTests, OrthographicProjectionMatchesDirectCall)
{
    Scene scene;
    Entity entity = scene.CreateEntity();

    CameraComponent& camera = entity.AddComponent<CameraComponent>();
    camera.projectionType = CameraProjectionType::Orthographic;
    camera.orthographicSize = 10.0f;
    camera.orthographicNear = -1.0f;
    camera.orthographicFar = 1.0f;
    camera.aspectRatio = 2.0f;

    CameraSystem::Update(scene);

    ExpectMatrixNear(camera.projection, Orthographic(-10.0f, 10.0f, -5.0f, 5.0f, -1.0f, 1.0f));
}
