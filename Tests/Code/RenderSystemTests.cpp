#include "pch.h"
#include "Graphics/RenderHandles.h"
#include "Graphics/Renderer.h"
#include "Graphics/ResourceManager.h"
#include "NullRendererAPI.h"
#include "Scene/Component/MaterialComponent.h"
#include "Scene/Component/MeshComponent.h"
#include "Scene/Scene.h"
#include "Scene/System/RenderSystem.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaTests;

// Regression test for a real crash: re-adding a Mesh component via the Inspector's "+ Add
// Component" leaves MeshComponent::mesh at its default (invalid) handle - RenderSystem::Draw used
// to submit it anyway, and Renderer::Flush() would then hit
// MT_ASSERT(mesh, "Submitted RenderData references an unknown mesh handle!"), which triggers
// __debugbreak() in a Debug build (not a catchable exception). Draw() now skips an entity whose
// mesh handle isn't valid instead of submitting it - this only needs to prove that skip happens,
// not exercise Flush() itself (which needs a real graphics context this test doesn't have).
TEST(RenderSystemTests, EntityWithUnassignedMeshHandleIsSkippedNotSubmitted)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);
    Renderer renderer(rendererAPI, resourceManager);

    Scene scene;

    // Scene::CreateEntity() already adds TransformComponent - RenderSystem::Draw's view needs one.
    Entity noMesh = scene.CreateEntity();
    noMesh.AddComponent<MaterialComponent>();
    noMesh.AddComponent<MeshComponent>();  // mesh left at its default - invalid - handle

    Entity withMesh = scene.CreateEntity();
    withMesh.AddComponent<MaterialComponent>();
    // RenderSystem::Draw only checks IsValid() - a self-contained check on the handle's id, not a
    // ResourceManager lookup - so an arbitrary non-zero handle exercises the "valid" path without
    // this test needing to actually create a mesh (CreateMesh isn't safe against NullRendererAPI -
    // see its own header comment).
    withMesh.AddComponent<MeshComponent>().mesh = MeshHandle(1);

    RenderSystem::Draw(scene, renderer);

    EXPECT_EQ(renderer.GetPendingRenderDataCount(), 1u);
}
