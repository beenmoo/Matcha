#include "pch.h"
#include "Core/Commands/AddScriptBindingCommand.h"
#include "Core/Commands/RemoveScriptBindingCommand.h"

#include "Graphics/ResourceManager.h"
#include "Scene/Component/PythonScriptComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"

#include "NullRendererAPI.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaEditor;
using namespace MatchaTests;

namespace
{
// bindings[i].moduleName/className as a "module.className" pair, for compact assertions on
// which scripts are bound and in what order - order matters here (unlike most of this codebase's
// component data) since PythonScriptSystem ticks bindings in vector order and two scripts on the
// same entity can genuinely produce different results depending on which runs first.
std::vector<std::string> BindingSummaries(Entity entity)
{
    std::vector<std::string> summaries;
    for (const PythonScriptComponent::Binding& binding : entity.GetComponent<PythonScriptComponent>().bindings)
        summaries.push_back(binding.moduleName + "." + binding.className);

    return summaries;
}
}  // namespace

// The bug this whole feature exists to fix: with only whole-component Add/RemoveComponentCommand,
// there was no way to remove a single script from an entity carrying several - only "remove the
// entire Python Script box," destroying every other binding along with it. AddScriptBindingCommand/
// RemoveScriptBindingCommand operate on one Binding within the vector instead.
TEST(ScriptBindingCommandTests, AddThenUndoThenRedoRestoresExactlyThatBinding)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);
    SceneManager sceneManager(resourceManager);

    Entity entity = sceneManager.GetScene().CreateEntity("Player");
    entity.AddComponent<PythonScriptComponent>();
    UUID id = entity.GetComponent<TagComponent>().id;

    AddScriptBindingCommand add(sceneManager, "Add Python Script", id, "rotation_component", "RotationComponent");
    add.Execute();

    ASSERT_EQ(BindingSummaries(entity), (std::vector<std::string>{"rotation_component.RotationComponent"}));

    add.Undo();
    EXPECT_TRUE(entity.GetComponent<PythonScriptComponent>().bindings.empty());

    add.Execute();
    EXPECT_EQ(BindingSummaries(entity), (std::vector<std::string>{"rotation_component.RotationComponent"}));
}

// The actual feature: two independent scripts on one entity, removing only one of them (via its
// own RemoveScriptBindingCommand) leaves the other completely untouched - the thing whole-component
// removal could never do.
TEST(ScriptBindingCommandTests, RemovingOneBindingLeavesTheOtherIntact)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);
    SceneManager sceneManager(resourceManager);

    Entity entity = sceneManager.GetScene().CreateEntity("Camera");
    PythonScriptComponent& script = entity.AddComponent<PythonScriptComponent>();
    script.Bind("flashlight", "Flashlight");
    script.Bind("camera_controller", "CameraController");
    UUID id = entity.GetComponent<TagComponent>().id;

    RemoveScriptBindingCommand remove(sceneManager, "Remove Python Script", id, 0);
    remove.Execute();

    EXPECT_EQ(BindingSummaries(entity), (std::vector<std::string>{"camera_controller.CameraController"}));

    remove.Undo();
    EXPECT_EQ(BindingSummaries(entity),
              (std::vector<std::string>{"flashlight.Flashlight", "camera_controller.CameraController"}));
}

// Undo has to put a removed binding back at its original position, not just append it - order is
// observable behavior here (PythonScriptSystem ticks in vector order), so silently reordering on
// undo would be a real, if subtle, behavior change rather than a cosmetic one.
TEST(ScriptBindingCommandTests, UndoRestoresARemovedMiddleBindingAtItsOriginalIndex)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);
    SceneManager sceneManager(resourceManager);

    Entity entity = sceneManager.GetScene().CreateEntity("Multi");
    PythonScriptComponent& script = entity.AddComponent<PythonScriptComponent>();
    script.Bind("a_module", "A");
    script.Bind("b_module", "B");
    script.Bind("c_module", "C");
    UUID id = entity.GetComponent<TagComponent>().id;

    RemoveScriptBindingCommand remove(sceneManager, "Remove Python Script", id, 1);  // removes B
    remove.Execute();
    ASSERT_EQ(BindingSummaries(entity), (std::vector<std::string>{"a_module.A", "c_module.C"}));

    remove.Undo();
    EXPECT_EQ(BindingSummaries(entity), (std::vector<std::string>{"a_module.A", "b_module.B", "c_module.C"}));
}

// Mirrors the actual CommandManager usage pattern: Add via the "Browse..." button, then Remove via
// the per-binding button, then undo both in the strict LIFO order CommandManager enforces. This is
// the interleaving AddScriptBindingCommand/RemoveScriptBindingCommand's own comments argue is safe
// specifically because of that LIFO guarantee - this test is what backs that argument up.
TEST(ScriptBindingCommandTests, InterleavedAddAndRemoveUndoRedoInStrictStackOrder)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);
    SceneManager sceneManager(resourceManager);

    Entity entity = sceneManager.GetScene().CreateEntity("Multi");
    PythonScriptComponent& script = entity.AddComponent<PythonScriptComponent>();
    script.Bind("a_module", "A");
    script.Bind("b_module", "B");
    UUID id = entity.GetComponent<TagComponent>().id;

    // Remove A (index 0), then add C (appended at the new end, index 1).
    RemoveScriptBindingCommand removeA(sceneManager, "Remove Python Script", id, 0);
    removeA.Execute();
    ASSERT_EQ(BindingSummaries(entity), (std::vector<std::string>{"b_module.B"}));

    AddScriptBindingCommand addC(sceneManager, "Add Python Script", id, "c_module", "C");
    addC.Execute();
    ASSERT_EQ(BindingSummaries(entity), (std::vector<std::string>{"b_module.B", "c_module.C"}));

    // Undo in strict LIFO order: addC first, then removeA.
    addC.Undo();
    EXPECT_EQ(BindingSummaries(entity), (std::vector<std::string>{"b_module.B"}));

    removeA.Undo();
    EXPECT_EQ(BindingSummaries(entity), (std::vector<std::string>{"a_module.A", "b_module.B"}));

    // Redo back in the same order.
    removeA.Execute();
    EXPECT_EQ(BindingSummaries(entity), (std::vector<std::string>{"b_module.B"}));

    addC.Execute();
    EXPECT_EQ(BindingSummaries(entity), (std::vector<std::string>{"b_module.B", "c_module.C"}));
}
