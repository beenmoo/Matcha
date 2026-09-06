#include "pch.h"
#include "Core/KeyCodes.h"
#include "Math/Quaternion.h"
#include "Scene/Component/LightComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"
#include "Scripting/PythonRuntime.h"

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

using namespace Matcha;

namespace
{
class TempDirectory
{
public:
    TempDirectory()
    {
        auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        m_Path = std::filesystem::temp_directory_path() / ("MatchaSandboxScriptsTest-" + std::to_string(unique));
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

// Runs Sandbox's actual, shipped scripts (not a paraphrased copy) against a hand-rolled fake
// EngineContext/Input written in Python - duck typing means these only need to implement the
// handful of methods each script actually calls, without a real C++ Input/EngineContext or the
// full Application dependency graph ScriptSystem's tests always lacked one for.
class SandboxScriptsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        runtime.RegisterScriptDirectory(SANDBOX_SCRIPTS_DIR);

        WriteFile(fixtureDir.GetPath() / "fakes.py",
                  "import matcha_engine\n"
                  "\n"
                  "class FakeTime:\n"
                  "    def __init__(self, delta_time):\n"
                  "        self.delta_time = delta_time\n"
                  "    def get_delta_time(self):\n"
                  "        return self.delta_time\n"
                  "\n"
                  "class FakeInput:\n"
                  "    def __init__(self, keys_held=()):\n"
                  "        self.keys_held = set(keys_held)\n"
                  "    def get_key(self, key):\n"
                  "        return key in self.keys_held\n"
                  "    def get_mouse_button(self, button):\n"
                  "        return False\n"
                  "    def get_mouse_button_down(self, button):\n"
                  "        return False\n"
                  "    def get_mouse_button_up(self, button):\n"
                  "        return False\n"
                  "    def get_axis(self, axis_type):\n"
                  "        return matcha_engine.Vector2Int(0, 0)\n"
                  "    def set_cursor_lock_state(self, state):\n"
                  "        pass\n"
                  "\n"
                  "class FakeContext:\n"
                  "    def __init__(self, scene=None, delta_time=1.0, keys_held=()):\n"
                  "        self.scene = scene\n"
                  "        self.time = FakeTime(delta_time)\n"
                  "        self.input = FakeInput(keys_held)\n"
                  "    def get_time(self):\n"
                  "        return self.time\n"
                  "    def get_input(self):\n"
                  "        return self.input\n"
                  "    def get_scene(self):\n"
                  "        return self.scene\n");
        runtime.RegisterScriptDirectory(fixtureDir.GetPath().string());
    }

    PythonRuntime runtime;
    TempDirectory fixtureDir;
};

// Instantiates `className` from `moduleName`, wires up entity/context the same way
// PythonScriptSystem::Update does, and returns the live instance so the test can call
// on_create()/on_update() and inspect whatever the script stored on itself (e.g. Flashlight's
// self.light).
py::object Instantiate(PythonRuntime& runtime, const std::string& moduleName, const std::string& className, Entity entity,
                        py::object context)
{
    py::module_ module = runtime.LoadScriptModule(moduleName);
    if (!module)
        return {};

    py::object cls = module.attr(className.c_str());
    py::object instance = cls();
    instance.attr("entity") = py::cast(entity);
    instance.attr("context") = std::move(context);
    return instance;
}
}  // namespace

TEST_F(SandboxScriptsTest, RotationComponentRotatesAroundYByDegreesPerSecondTimesDeltaTime)
{
    Scene scene;
    Entity entity = scene.CreateEntity("Cube");

    py::module_ fakes = runtime.LoadScriptModule("fakes");
    ASSERT_TRUE(static_cast<bool>(fakes));
    py::object context = fakes.attr("FakeContext")(py::none(), 1.0f);

    py::object instance = Instantiate(runtime, "rotation_component", "RotationComponent", entity, context);
    ASSERT_TRUE(static_cast<bool>(instance));

    instance.attr("on_update")();

    // 45 degrees_per_second * 1.0s delta time, around +Y, Self space (Transform::Rotate's
    // default) - matches AngleAxis(Radians(45), Y) applied to an identity starting rotation.
    Quaternion expected = AngleAxis(Radians(45.0f), Vector3(0.0f, 1.0f, 0.0f));
    const Quaternion& actual = entity.GetComponent<TransformComponent>().transform.GetRotation();

    EXPECT_NEAR(actual.x, expected.x, 0.001f);
    EXPECT_NEAR(actual.y, expected.y, 0.001f);
    EXPECT_NEAR(actual.z, expected.z, 0.001f);
    EXPECT_NEAR(actual.w, expected.w, 0.001f);
}

TEST_F(SandboxScriptsTest, FlashlightCreatesASpotLightAndFollowsItsHost)
{
    Scene scene;
    Entity host = scene.CreateEntity("Camera");
    host.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 2.0f, 3.0f);

    py::module_ fakes = runtime.LoadScriptModule("fakes");
    ASSERT_TRUE(static_cast<bool>(fakes));
    py::object context = fakes.attr("FakeContext")(py::cast(&scene), 1.0f);

    py::object instance = Instantiate(runtime, "flashlight", "Flashlight", host, context);
    ASSERT_TRUE(static_cast<bool>(instance));

    instance.attr("on_create")();

    Entity light = instance.attr("light").cast<Entity>();
    ASSERT_TRUE(light.IsValid());
    ASSERT_TRUE(light.HasComponent<LightComponent>());

    const LightComponent& lightComponent = light.GetComponent<LightComponent>();
    EXPECT_EQ(lightComponent.type, LightType::Spot);
    EXPECT_FLOAT_EQ(lightComponent.intensity, 5.0f);

    instance.attr("on_update")();

    const Vector3& lightPosition = light.GetComponent<TransformComponent>().transform.GetPosition();
    EXPECT_FLOAT_EQ(lightPosition.x, 1.0f);
    EXPECT_FLOAT_EQ(lightPosition.y, 2.0f);
    EXPECT_FLOAT_EQ(lightPosition.z, 3.0f);
}

TEST_F(SandboxScriptsTest, CameraControllerMovesForwardWhenWIsHeld)
{
    Scene scene;
    Entity camera = scene.CreateEntity("Camera");
    Vector3 startPosition = camera.GetComponent<TransformComponent>().transform.GetPosition();

    py::module_ fakes = runtime.LoadScriptModule("fakes");
    ASSERT_TRUE(static_cast<bool>(fakes));
    py::list keysHeld;
    keysHeld.append(KeyCode::W);
    py::object context = fakes.attr("FakeContext")(py::none(), 1.0f, keysHeld);

    py::object instance = Instantiate(runtime, "camera_controller", "CameraController", camera, context);
    ASSERT_TRUE(static_cast<bool>(instance));

    instance.attr("on_update")();

    // Convention-agnostic: rather than assume which axis "forward" is, just confirm the entity
    // actually moved by exactly MOVE_SPEED (2.0) * delta_time (1.0) - the magnitude the
    // get_key(W) -> get_forward() -> normalize() -> translate() chain should produce.
    const Vector3& endPosition = camera.GetComponent<TransformComponent>().transform.GetPosition();
    Vector3 delta = endPosition - startPosition;
    float distanceMoved = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    EXPECT_NEAR(distanceMoved, 2.0f, 0.001f);
}
