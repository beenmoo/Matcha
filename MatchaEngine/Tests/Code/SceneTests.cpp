#include "pch.h"
#include "Scene/Scene.h"

#include <gtest/gtest.h>

using namespace Matcha;

namespace
{
struct PositionComponent
{
    float x = 0.0f;
    float y = 0.0f;
};
}  // namespace

TEST(SceneTests, CreateEntityIsValid)
{
    Scene scene;
    Entity entity = scene.CreateEntity();

    EXPECT_TRUE(entity.IsValid());
    EXPECT_TRUE(static_cast<bool>(entity));
}

TEST(SceneTests, DefaultConstructedEntityIsInvalid)
{
    Entity entity;

    EXPECT_FALSE(entity.IsValid());
}

TEST(SceneTests, AddAndGetComponent)
{
    Scene scene;
    Entity entity = scene.CreateEntity();

    entity.AddComponent<PositionComponent>(1.0f, 2.0f);

    EXPECT_TRUE(entity.HasComponent<PositionComponent>());

    const PositionComponent& pos = entity.GetComponent<PositionComponent>();
    EXPECT_FLOAT_EQ(pos.x, 1.0f);
    EXPECT_FLOAT_EQ(pos.y, 2.0f);
}

TEST(SceneTests, RemoveComponent)
{
    Scene scene;
    Entity entity = scene.CreateEntity();

    entity.AddComponent<PositionComponent>();
    entity.RemoveComponent<PositionComponent>();

    EXPECT_FALSE(entity.HasComponent<PositionComponent>());
}

TEST(SceneTests, DestroyEntityInvalidatesIt)
{
    Scene scene;
    Entity entity = scene.CreateEntity();

    scene.DestroyEntity(entity);

    EXPECT_FALSE(entity.IsValid());
}

TEST(SceneTests, ViewIteratesEntitiesWithComponent)
{
    Scene scene;

    Entity a = scene.CreateEntity();
    a.AddComponent<PositionComponent>(1.0f, 0.0f);

    Entity b = scene.CreateEntity();
    b.AddComponent<PositionComponent>(2.0f, 0.0f);

    (void)scene.CreateEntity();  // no PositionComponent, shouldn't show up in the view

    int count = 0;
    float sumX = 0.0f;

    for (auto [handle, pos] : scene.View<PositionComponent>().each())
    {
        ++count;
        sumX += pos.x;
    }

    EXPECT_EQ(count, 2);
    EXPECT_FLOAT_EQ(sumX, 3.0f);
}
