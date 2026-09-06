#pragma once

#include <pybind11/embed.h>
#include <string>

namespace py = pybind11;

namespace Matcha
{
class PythonRuntime
{
public:
    PythonRuntime();
    ~PythonRuntime() = default;
    PythonRuntime(const PythonRuntime&) = delete;
    PythonRuntime& operator=(const PythonRuntime&) = delete;

    void RegisterScriptDirectory(const std::string& directory);
    [[nodiscard]] py::module_ LoadScriptModule(const std::string& moduleName);

private:
    py::scoped_interpreter m_Interpreter;
};
}  // namespace Matcha