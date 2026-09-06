#include "pch.h"
#include "Scripting/PythonRuntime.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

using namespace Matcha;

namespace
{
// Creates a unique scratch directory for a test and removes it (and its contents) on teardown -
// same shape as FileWatcherTests.cpp's TempDirectory.
class TempDirectory
{
public:
    TempDirectory()
    {
        auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        m_Path = std::filesystem::temp_directory_path() / ("MatchaPythonRuntimeTest-" + std::to_string(unique));
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

// Both cases share a single PythonRuntime (one scoped_interpreter init/finalize cycle for the
// whole file) rather than one per TEST() - repeated Py_Initialize/Py_Finalize cycles within one
// process are a known rough edge for some Python C extensions, and there's no reason to risk that
// here when both cases are cheap to run against one already-live interpreter.
TEST(PythonRuntimeTests, RegistersDirectoryAndLoadsScripts)
{
    PythonRuntime runtime;

    TempDirectory dir;
    WriteFile(dir.GetPath() / "greeter.py",
              "def add(a, b):\n"
              "    return a + b\n");
    runtime.RegisterScriptDirectory(dir.GetPath().string());

    py::module_ module = runtime.LoadScriptModule("greeter");
    ASSERT_TRUE(static_cast<bool>(module));

    int result = module.attr("add")(2, 3).cast<int>();
    EXPECT_EQ(result, 5);

    // An unregistered/unknown module name should come back as a loggable, default-constructed
    // (null-handle) module, not a thrown pybind11::error_already_set escaping into the caller -
    // see PythonRuntime.cpp's catch block. Note this is a null handle (ptr() == nullptr), not
    // Python's None singleton, so the check is bool-conversion/ptr(), not is_none().
    py::module_ missing = runtime.LoadScriptModule("ThisModuleDoesNotExist");
    EXPECT_FALSE(static_cast<bool>(missing));
}
