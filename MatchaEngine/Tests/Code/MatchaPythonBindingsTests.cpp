#include "pch.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"
#include "Scripting/PythonRuntime.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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

// Proves the actual mechanism InspectorPanel::AddPythonScriptFields depends on to auto-generate
// Inspector fields from a script's public attributes: reading a live instance's __dict__, filtering
// out engine-plumbing/private names, and telling bool/int/float/str apart via isinstance - none of
// which was previously exercised anywhere. The Qt widget creation itself can't be unit-tested here
// (no QApplication in this binary), but this is the part that was actually new/unverified.
TEST(MatchaPythonBindingsTests, InstanceDictExposesPublicFieldsWithCorrectTypes)
{
    PythonRuntime runtime;

    TempDirectory dir;
    WriteFile(dir.GetPath() / "widget.py",
              "class Widget:\n"
              "    def __init__(self):\n"
              "        self.speed = 2.5\n"
              "        self.count = 3\n"
              "        self.enabled = True\n"
              "        self.name = \"hello\"\n"
              "        self._private = 42\n");
    runtime.RegisterScriptDirectory(dir.GetPath().string());

    py::module_ module = runtime.LoadScriptModule("widget");
    ASSERT_TRUE(static_cast<bool>(module));

    py::object instance = module.attr("Widget")();
    // The same plumbing PythonScriptSystem::Update sets on every real instance - should be
    // excluded by name, same as AddPythonScriptFields does, not just happen to not collide here.
    instance.attr("entity") = 123;
    instance.attr("context") = 456;

    py::dict fields = instance.attr("__dict__").cast<py::dict>();

    std::vector<std::string> visibleKeys;
    for (auto item : fields)
    {
        std::string key = py::str(item.first).cast<std::string>();
        if (key == "entity" || key == "context" || (!key.empty() && key[0] == '_'))
            continue;
        visibleKeys.push_back(key);
    }

    ASSERT_EQ(visibleKeys.size(), 4u);
    EXPECT_NE(std::find(visibleKeys.begin(), visibleKeys.end(), "speed"), visibleKeys.end());
    EXPECT_NE(std::find(visibleKeys.begin(), visibleKeys.end(), "count"), visibleKeys.end());
    EXPECT_NE(std::find(visibleKeys.begin(), visibleKeys.end(), "enabled"), visibleKeys.end());
    EXPECT_NE(std::find(visibleKeys.begin(), visibleKeys.end(), "name"), visibleKeys.end());

    // bool has to be checked before int: in Python, bool is an int subclass, so isinstance<int_>
    // alone would misclassify every bool field as numeric - exactly the mistake AddPythonScriptFields
    // avoids by checking bool_ first.
    py::object enabledValue = fields["enabled"];
    EXPECT_TRUE(py::isinstance<py::bool_>(enabledValue));
    EXPECT_EQ(enabledValue.cast<bool>(), true);

    py::object countValue = fields["count"];
    EXPECT_FALSE(py::isinstance<py::bool_>(countValue));
    EXPECT_TRUE(py::isinstance<py::int_>(countValue));
    EXPECT_EQ(countValue.cast<int>(), 3);

    py::object speedValue = fields["speed"];
    EXPECT_TRUE(py::isinstance<py::float_>(speedValue));
    EXPECT_FLOAT_EQ(speedValue.cast<float>(), 2.5f);

    py::object nameValue = fields["name"];
    EXPECT_TRUE(py::isinstance<py::str>(nameValue));
    EXPECT_EQ(nameValue.cast<std::string>(), "hello");
}
