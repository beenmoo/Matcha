#include "pch.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Scene.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

using namespace Matcha;

namespace
{
// Walks the firstChild -> nextSibling chain to collect a parent's children for assertions.
std::vector<Entity> GetChildren(Entity parent)
{
    std::vector<Entity> children;

    if (!parent.HasComponent<HierarchyComponent>())
        return children;

    entt::entity handle = parent.GetComponent<HierarchyComponent>().firstChild;

    while (handle != entt::null)
    {
        Entity child = parent.WithHandle(handle);
        children.push_back(child);
        handle = child.GetComponent<HierarchyComponent>().nextSibling;
    }

    return children;
}
}  // namespace

TEST(HierarchyComponentTests, SetParentLinksBothSides)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    SetParent(child, parent);

    EXPECT_EQ(child.GetComponent<HierarchyComponent>().parent, parent.GetHandle());
    EXPECT_EQ(GetChildren(parent), std::vector<Entity>{child});
}

TEST(HierarchyComponentTests, AddsComponentAutomaticallyIfMissing)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    EXPECT_FALSE(child.HasComponent<HierarchyComponent>());
    EXPECT_FALSE(parent.HasComponent<HierarchyComponent>());

    SetParent(child, parent);

    EXPECT_TRUE(child.HasComponent<HierarchyComponent>());
    EXPECT_TRUE(parent.HasComponent<HierarchyComponent>());
}

TEST(HierarchyComponentTests, MultipleChildrenAllAppearUnderParent)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity childA = scene.CreateEntity();
    Entity childB = scene.CreateEntity();

    SetParent(childA, parent);
    SetParent(childB, parent);

    std::vector<Entity> children = GetChildren(parent);
    EXPECT_EQ(children.size(), 2u);
    EXPECT_NE(std::find(children.begin(), children.end(), childA), children.end());
    EXPECT_NE(std::find(children.begin(), children.end(), childB), children.end());
}

TEST(HierarchyComponentTests, ReparentingMovesChildOutOfOldParent)
{
    Scene scene;
    Entity oldParent = scene.CreateEntity();
    Entity newParent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    SetParent(child, oldParent);
    SetParent(child, newParent);

    EXPECT_EQ(child.GetComponent<HierarchyComponent>().parent, newParent.GetHandle());
    EXPECT_TRUE(GetChildren(oldParent).empty());
    EXPECT_EQ(GetChildren(newParent), std::vector<Entity>{child});
}

TEST(HierarchyComponentTests, DetachingClearsParentAndRemovesFromChildren)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    SetParent(child, parent);
    SetParent(child, Entity());

    EXPECT_TRUE(child.GetComponent<HierarchyComponent>().parent == entt::null);
    EXPECT_TRUE(GetChildren(parent).empty());
}

TEST(HierarchyComponentTests, RemovingMiddleChildRelinksRemainingSiblings)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity a = scene.CreateEntity();
    Entity b = scene.CreateEntity();
    Entity c = scene.CreateEntity();

    // Each SetParent inserts at the head, so the list is now c -> b -> a.
    SetParent(a, parent);
    SetParent(b, parent);
    SetParent(c, parent);

    ASSERT_EQ(GetChildren(parent), (std::vector<Entity>{c, b, a}));

    // b has both a previous and a next sibling: the trickiest unlink case.
    SetParent(b, Entity());

    EXPECT_EQ(GetChildren(parent), (std::vector<Entity>{c, a}));
    EXPECT_TRUE(b.GetComponent<HierarchyComponent>().parent == entt::null);
}

TEST(HierarchyComponentTests, ChildrenCountTracksAttachAndDetach)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity a = scene.CreateEntity();
    Entity b = scene.CreateEntity();

    SetParent(a, parent);
    SetParent(b, parent);

    EXPECT_EQ(parent.GetComponent<HierarchyComponent>().childrenCount, 2u);

    SetParent(a, Entity());

    EXPECT_EQ(parent.GetComponent<HierarchyComponent>().childrenCount, 1u);
}

TEST(HierarchyComponentTests, ChildrenCountUpdatesOnReparent)
{
    Scene scene;
    Entity oldParent = scene.CreateEntity();
    Entity newParent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    SetParent(child, oldParent);
    SetParent(child, newParent);

    EXPECT_EQ(oldParent.GetComponent<HierarchyComponent>().childrenCount, 0u);
    EXPECT_EQ(newParent.GetComponent<HierarchyComponent>().childrenCount, 1u);
}

TEST(HierarchyComponentTests, DestroyEntityRecursiveWithNoHierarchyJustDestroysIt)
{
    Scene scene;
    Entity entity = scene.CreateEntity();

    DestroyEntityRecursive(scene, entity);

    EXPECT_FALSE(entity.IsValid());
}

TEST(HierarchyComponentTests, DestroyEntityRecursiveRemovesLeafFromParentButKeepsParentAlive)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity child = scene.CreateEntity();

    SetParent(child, parent);

    DestroyEntityRecursive(scene, child);

    EXPECT_FALSE(child.IsValid());
    EXPECT_TRUE(parent.IsValid());
    EXPECT_TRUE(GetChildren(parent).empty());
    EXPECT_EQ(parent.GetComponent<HierarchyComponent>().childrenCount, 0u);
}

TEST(HierarchyComponentTests, DestroyEntityRecursiveDestroysWholeSubtree)
{
    Scene scene;
    Entity root = scene.CreateEntity();
    Entity child = scene.CreateEntity();
    Entity grandchild = scene.CreateEntity();

    SetParent(child, root);
    SetParent(grandchild, child);

    DestroyEntityRecursive(scene, root);

    EXPECT_FALSE(root.IsValid());
    EXPECT_FALSE(child.IsValid());
    EXPECT_FALSE(grandchild.IsValid());
}

TEST(HierarchyComponentTests, DestroyEntityRecursiveLeavesSiblingSubtreeIntact)
{
    Scene scene;
    Entity parent = scene.CreateEntity();
    Entity doomed = scene.CreateEntity();
    Entity doomedChild = scene.CreateEntity();
    Entity survivor = scene.CreateEntity();

    SetParent(doomed, parent);
    SetParent(doomedChild, doomed);
    SetParent(survivor, parent);

    DestroyEntityRecursive(scene, doomed);

    EXPECT_FALSE(doomed.IsValid());
    EXPECT_FALSE(doomedChild.IsValid());
    EXPECT_TRUE(survivor.IsValid());
    EXPECT_EQ(GetChildren(parent), std::vector<Entity>{survivor});
    EXPECT_EQ(parent.GetComponent<HierarchyComponent>().childrenCount, 1u);
}
