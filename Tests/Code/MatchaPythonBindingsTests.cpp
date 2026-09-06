#include "pch.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"
#include "Scripting/PythonRuntime.h"

#include <gtest/gtest.h>

#include <chrono>
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
        m_Path = std::filesystem::temp_directory_path() / ("MatchaPythonBindingsTest-" + std::to_string(unique));
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

// Proves the matcha_engine embedded module (Entity/Transform/Vector3 bindings in
// MatchaPythonBindings.cpp) actually round-trips into real engine state, not just that it
// compiles - a script reads an entity's Transform, writes a new position through it, and this
// checks the change landed on the same TransformComponent the C++ side sees.
TEST(MatchaPythonBindingsTests, ScriptCanReadAndWriteAnEntitysTransform)
{
    Scene scene;
    Entity entity = scene.CreateEntity("Mover");
    entity.GetComponent<TransformComponent>().transform.SetPosition(1.0f, 2.0f, 3.0f);

    PythonRuntime runtime;

    TempDirectory dir;
    WriteFile(dir.GetPath() / "mover.py",
              "import matcha_engine\n"
              "\n"
              "def move(entity):\n"
              "    t = entity.get_transform()\n"
              "    pos = t.get_position()\n"
              "    t.set_position(matcha_engine.Vector3(pos.x + 10.0, pos.y, pos.z))\n");
    runtime.RegisterScriptDirectory(dir.GetPath().string());

    py::module_ module = runtime.LoadScriptModule("mover");
    ASSERT_TRUE(static_cast<bool>(module));

    module.attr("move")(py::cast(entity));

    const Vector3& position = entity.GetComponent<TransformComponent>().transform.GetPosition();
    EXPECT_FLOAT_EQ(position.x, 11.0f);
    EXPECT_FLOAT_EQ(position.y, 2.0f);
    EXPECT_FLOAT_EQ(position.z, 3.0f);
}
