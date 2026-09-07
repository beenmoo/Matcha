#include "pch.h"
#include "Scene/Component/LightComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"
#include "Scripting/PythonRuntime.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

using namespace Matcha;

namespace
{
class TempDirectory
{
public:
    TempDirectory()
    {
        auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        m_Path = std::filesystem::temp_directory_path() / ("MatchaStaleEntity-" + std::to_string(unique));
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
}  // namespace

// A script holding an Entity across frames (Flashlight caches the light entity it spawns) has no
// say in whether that entity survives - the editor's Scene Hierarchy can delete it at any point.
// Before the accessors checked validity, the next on_update() reached into a dead handle and hit
// entt's own "Set does not contain entity" assertion inside registry.get<T>(), taking the editor
// down with nothing pointing at which script was responsible.
//
// A raised Python exception is the whole point: PythonScriptSystem catches py::error_already_set
// per binding, so this turns a process-killing assertion into one logged line naming the script.
TEST(StaleEntityBindingTests, TouchingADeletedEntityRaisesInsteadOfCrashing)
{
    PythonRuntime runtime;

    Scene scene;
    Entity entity = scene.CreateEntity("Doomed");

    TempDirectory dir;
    // Importing matcha_engine is what registers the Entity type with pybind11 - py::cast(entity)
    // below fails with "Unregistered type" until some script has done it, exactly as a real
    // script does.
    WriteFile(dir.GetPath() / "toucher.py",
              "import matcha_engine\n"
              "\n"
              "def read_transform(entity):\n"
              "    return entity.get_transform().get_position()\n"
              "\n"
              "def add_light(entity):\n"
              "    return entity.add_light_component()\n");
    runtime.RegisterScriptDirectory(dir.GetPath().string());

    py::module_ module = runtime.LoadScriptModule("toucher");
    ASSERT_TRUE(static_cast<bool>(module));

    // Same handle the script would have cached, still live at this point.
    py::object cached = py::cast(entity);
    EXPECT_NO_THROW(module.attr("read_transform")(cached));

    scene.DestroyEntity(entity);

    EXPECT_THROW(module.attr("read_transform")(cached), py::error_already_set);
    EXPECT_THROW(module.attr("add_light")(cached), py::error_already_set);
}

// The guard a script is expected to use before touching an entity it has held onto - Flashlight's
// on_update() checks exactly this. Has to answer correctly on a dead handle rather than asserting
// on the way to the answer, which is why it goes through Scene's registry.valid() and not
// all_of<T>() (entt asserts on an invalid entity there too).
TEST(StaleEntityBindingTests, IsValidReportsWhetherTheEntityStillExists)
{
    PythonRuntime runtime;

    Scene scene;
    Entity entity = scene.CreateEntity("Doomed");

    TempDirectory dir;
    WriteFile(dir.GetPath() / "checker.py",
              "import matcha_engine\n"
              "\n"
              "def still_there(entity):\n"
              "    return entity.is_valid()\n");
    runtime.RegisterScriptDirectory(dir.GetPath().string());

    py::module_ module = runtime.LoadScriptModule("checker");
    ASSERT_TRUE(static_cast<bool>(module));

    py::object cached = py::cast(entity);
    EXPECT_TRUE(module.attr("still_there")(cached).cast<bool>());

    scene.DestroyEntity(entity);

    EXPECT_FALSE(module.attr("still_there")(cached).cast<bool>());
}

// Adding a light twice used to reach entt's emplace<T>, which asserts rather than replacing - a
// plain script bug (or a re-run on_create) shouldn't be able to kill the process either.
TEST(StaleEntityBindingTests, AddingALightTwiceRaisesInsteadOfAsserting)
{
    PythonRuntime runtime;

    Scene scene;
    Entity entity = scene.CreateEntity("Lit");

    TempDirectory dir;
    WriteFile(dir.GetPath() / "adder.py",
              "import matcha_engine\n"
              "\n"
              "def add_light(entity):\n"
              "    return entity.add_light_component()\n");
    runtime.RegisterScriptDirectory(dir.GetPath().string());

    py::module_ module = runtime.LoadScriptModule("adder");
    ASSERT_TRUE(static_cast<bool>(module));

    py::object cached = py::cast(entity);
    EXPECT_NO_THROW(module.attr("add_light")(cached));
    ASSERT_TRUE(entity.HasComponent<LightComponent>());

    EXPECT_THROW(module.attr("add_light")(cached), py::error_already_set);

    // The first light is still intact - the rejected second add didn't disturb it.
    EXPECT_TRUE(entity.HasComponent<LightComponent>());
}
