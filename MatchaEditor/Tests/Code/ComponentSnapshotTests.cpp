#include "pch.h"
#include "Core/Commands/ComponentSnapshot.h"

#include "Graphics/ResourceManager.h"
#include "Scene/Component/PythonScriptComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Scene.h"
#include "Scripting/PythonRuntime.h"

#include "NullRendererAPI.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

using namespace Matcha;
using namespace MatchaEditor;
using namespace MatchaTests;

namespace
{
class TempDirectory
{
public:
    TempDirectory()
    {
        auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        m_Path = std::filesystem::temp_directory_path() / ("MatchaComponentSnapshot-" + std::to_string(unique));
        std::filesystem::create_directories(m_Path);
    }

    ~TempDirectory()
    {
        std::error_code ec;
        std::filesystem::remove_all(m_Path, ec);
    }

    [[nodiscard]] const std::filesystem::path& GetPath() const
    {
        return m_Path;
    }

private:
    std::filesystem::path m_Path;
};

void WriteFile(const std::filesystem::path& path, std::string_view contents)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << contents;
}

// Mirrors PythonScriptSystem::Update's per-binding body (lazy instantiate + on_create, then
// on_update) - inlined here because that function needs a real Input/Time, which can't be built
// in this test binary (Application's constructor unconditionally opens a real OS window).
void TickBinding(PythonScriptComponent::Binding& binding, PythonRuntime& runtime, Entity entity, py::object input,
                 py::object time, py::object scene)
{
    if (!binding.instance || binding.instance.is_none())
    {
        py::module_ module = runtime.LoadScriptModule(binding.moduleName);
        ASSERT_TRUE(static_cast<bool>(module));
        py::object cls = module.attr(binding.className.c_str());
        binding.instance = cls();
        binding.instance.attr("entity") = py::cast(entity);
        binding.instance.attr("input") = input;
        binding.instance.attr("time") = time;
        binding.instance.attr("scene") = scene;

        if (py::hasattr(binding.instance, "on_create"))
            binding.instance.attr("on_create")();
    }

    if (py::hasattr(binding.instance, "on_update"))
        binding.instance.attr("on_update")();
}

// Stands in for the engine's real Input/Time, which the scripts under test reach through
// self.input/self.time. Same shape as SandboxScriptsTests.cpp's own fixture.
constexpr std::string_view kFakesModule =
    "import matcha_engine\n"
    "\n"
    "class FakeTime:\n"
    "    def get_delta_time(self):\n"
    "        return 1.0 / 60.0\n"
    "\n"
    "class FakeInput:\n"
    "    def get_key(self, key):\n"
    "        return False\n"
    "    def get_mouse_button(self, button):\n"
    "        return False\n"
    "    def get_mouse_button_down(self, button):\n"
    "        return False\n"
    "    def get_mouse_button_up(self, button):\n"
    "        return False\n"
    "    def get_axis(self, axis_type):\n"
    "        return matcha_engine.Vector2Int(0, 0)\n"
    "    def set_cursor_lock_state(self, state):\n"
    "        pass\n";
}  // namespace

// The bug this covers: adding a Python Script component, binding a script to it, undoing, then
// redoing used to bring the component back empty - the script was gone. AddComponentCommand
// re-added a blank default on redo, because the binding was added after the component (via
// Browse...), so nothing in the command knew about it. It now captures on Undo() and replays that
// capture on Execute(), which is exactly the sequence below.
TEST(ComponentSnapshotTests, RedoOfAnAddRestoresAScriptBoundAfterTheComponentWasAdded)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);

    Scene scene;
    Entity entity = scene.CreateEntity("Player");
    UUID id = entity.GetComponent<TagComponent>().id;

    // 1. "+ Add Component" -> AddComponentCommand::Execute() with a snapshot it has never filled
    // in yet, which has to come out as a blank component.
    RestoreComponent<PythonScriptComponent>(entity, nlohmann::json(), "pythonScript", resourceManager);
    ASSERT_TRUE(entity.HasComponent<PythonScriptComponent>());
    ASSERT_TRUE(entity.GetComponent<PythonScriptComponent>().bindings.empty());

    // 2. "Browse..." -> the user picks a script. Not itself an undoable command, which is the
    // whole reason the add command has to preserve it.
    entity.GetComponent<PythonScriptComponent>().Bind("rotation_component", "RotationComponent");

    // 3. Ctrl+Z -> AddComponentCommand::Undo().
    nlohmann::json snapshot = CaptureComponent(entity, "pythonScript", resourceManager);
    entity.RemoveComponent<PythonScriptComponent>();
    ASSERT_FALSE(scene.FindEntityByUUID(id).HasComponent<PythonScriptComponent>());

    // 4. Ctrl+Shift+Z -> AddComponentCommand::Execute() again, this time with a real snapshot.
    Entity redone = scene.FindEntityByUUID(id);
    RestoreComponent<PythonScriptComponent>(redone, snapshot, "pythonScript", resourceManager);

    const PythonScriptComponent& restored = scene.FindEntityByUUID(id).GetComponent<PythonScriptComponent>();
    ASSERT_EQ(restored.bindings.size(), 1u);
    EXPECT_EQ(restored.bindings.front().moduleName, "rotation_component");
    EXPECT_EQ(restored.bindings.front().className, "RotationComponent");
}

// A component that writes nothing under its own key - a PythonScriptComponent with no bindings is
// the case that actually shows up, since ComponentRegistry skips the key entirely rather than
// writing an empty array. Restoring has to fall back to a blank default; the failure mode this
// guards against is undo/redo silently leaving the component off the entity altogether.
TEST(ComponentSnapshotTests, ACaptureThatWroteNothingStillRestoresTheComponent)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);

    Scene scene;
    Entity entity = scene.CreateEntity("Empty");
    UUID id = entity.GetComponent<TagComponent>().id;
    entity.AddComponent<PythonScriptComponent>();

    nlohmann::json snapshot = CaptureComponent(entity, "pythonScript", resourceManager);
    EXPECT_FALSE(snapshot.contains("pythonScript"));

    entity.RemoveComponent<PythonScriptComponent>();
    ASSERT_FALSE(scene.FindEntityByUUID(id).HasComponent<PythonScriptComponent>());

    Entity resolved = scene.FindEntityByUUID(id);
    RestoreComponent<PythonScriptComponent>(resolved, snapshot, "pythonScript", resourceManager);

    Entity restored = scene.FindEntityByUUID(id);
    ASSERT_TRUE(restored.HasComponent<PythonScriptComponent>());
    EXPECT_TRUE(restored.GetComponent<PythonScriptComponent>().bindings.empty());
}

// An unregistered component key (nothing in ComponentRegistry answers to it) is the other way
// CaptureComponent can come back with nothing - same fallback, but reached through
// FindComponentSerializer returning null rather than through write() declining to write.
TEST(ComponentSnapshotTests, AnUnregisteredComponentKeyStillRestoresABlankComponent)
{
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);

    Scene scene;
    Entity entity = scene.CreateEntity("Player");

    EXPECT_EQ(FindComponentSerializer("notARegisteredKey"), nullptr);

    nlohmann::json snapshot = CaptureComponent(entity, "notARegisteredKey", resourceManager);
    EXPECT_TRUE(snapshot.is_null());

    RestoreComponent<PythonScriptComponent>(entity, snapshot, "notARegisteredKey", resourceManager);
    EXPECT_TRUE(entity.HasComponent<PythonScriptComponent>());
}

// The full Remove -> Undo cycle for an entity whose scripts are actually running, mirroring
// Sandbox's real camera entity (two bindings on one entity). A snapshot that round-trips as JSON
// but comes back as bindings the runtime can't instantiate isn't a working undo, so this ticks the
// restored bindings the way PythonScriptSystem would on the frames after the undo.
TEST(ComponentSnapshotTests, RestoredBindingsAreStillRunnableForATwoScriptEntity)
{
    // PythonRuntime declared before Scene deliberately: local destruction runs in reverse
    // declaration order, and every PythonScriptComponent (owned transitively by Scene's registry)
    // must be destroyed before the interpreter it needs to release its pybind11::object instances
    // is gone - see PythonScriptComponent.h's own comment on this. Getting this backwards in an
    // earlier version of this test produced a real crash on teardown ("Fatal Python error: ... the
    // GIL is released ... Python runtime state: finalizing"), so the order here is load-bearing.
    NullRendererAPI rendererAPI;
    ResourceManager resourceManager(rendererAPI);
    PythonRuntime runtime;
    runtime.RegisterScriptDirectory(SANDBOX_SCRIPTS_DIR);

    Scene scene;
    Entity camera = scene.CreateEntity("Camera");
    UUID cameraId = camera.GetComponent<TagComponent>().id;

    PythonScriptComponent& script = camera.AddComponent<PythonScriptComponent>();
    script.Bind("flashlight", "Flashlight");
    script.Bind("camera_controller", "CameraController");

    TempDirectory fixtureDir;
    WriteFile(fixtureDir.GetPath() / "fakes.py", kFakesModule);
    runtime.RegisterScriptDirectory(fixtureDir.GetPath().string());

    py::module_ fakes = runtime.LoadScriptModule("fakes");
    ASSERT_TRUE(static_cast<bool>(fakes));
    py::object input = fakes.attr("FakeInput")();
    py::object time = fakes.attr("FakeTime")();
    py::object sceneObj = py::cast(&scene);

    // Get both scripts actually running - Flashlight's on_create() spawns its own light entity.
    for (PythonScriptComponent::Binding& binding : camera.GetComponent<PythonScriptComponent>().bindings)
        TickBinding(binding, runtime, camera, input, time, sceneObj);

    ASSERT_EQ(scene.GetRootEntities().size(), 2u);  // Camera + Flashlight's spawned light

    // RemoveComponentCommand's constructor, then Execute() - resolved fresh by UUID, matching how
    // the real command always re-resolves rather than holding a raw Entity handle.
    nlohmann::json snapshot;
    {
        Entity resolved = scene.FindEntityByUUID(cameraId);
        snapshot = CaptureComponent(resolved, "pythonScript", resourceManager);
        resolved.RemoveComponent<PythonScriptComponent>();
    }
    ASSERT_TRUE(snapshot.contains("pythonScript"));
    ASSERT_FALSE(scene.FindEntityByUUID(cameraId).HasComponent<PythonScriptComponent>());

    // Undo().
    {
        Entity resolved = scene.FindEntityByUUID(cameraId);
        RestoreComponent<PythonScriptComponent>(resolved, snapshot, "pythonScript", resourceManager);
    }

    Entity restoredCamera = scene.FindEntityByUUID(cameraId);
    ASSERT_TRUE(restoredCamera.HasComponent<PythonScriptComponent>());
    ASSERT_EQ(restoredCamera.GetComponent<PythonScriptComponent>().bindings.size(), 2u);

    // The next two frames after the undo: both bindings re-instantiate from scratch (a restored
    // Binding::instance is empty) and run on_create() again, then settle into on_update only.
    for (int frame = 0; frame < 2; ++frame)
        for (PythonScriptComponent::Binding& binding : restoredCamera.GetComponent<PythonScriptComponent>().bindings)
            TickBinding(binding, runtime, restoredCamera, input, time, sceneObj);

    SUCCEED();
}
