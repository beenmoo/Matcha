#include "PythonRuntime.h"

#include "Core/Logger.h"
#include "MatchaPythonBindings.h"

namespace Matcha
{
PythonRuntime::PythonRuntime()
    : m_Interpreter()
{
    ForceLinkMatchaPythonBindings();
}

void PythonRuntime::RegisterScriptDirectory(const std::string& directory)
{
    try
    {
        py::module_ sys = py::module_::import("sys");
        py::list sysPath = sys.attr("path");
        sysPath.attr("append")(directory);

        MT_CORE_INFO("Added script directory: {}", directory);
    }
    catch (const py::error_already_set& e)
    {
        MT_CORE_ERROR("Failed to add script directory: {}", e.what());
    }
}

py::module_ PythonRuntime::LoadScriptModule(const std::string& moduleName)
{
    try
    {
        return py::module_::import(moduleName.c_str());
    }
    catch (const py::error_already_set& e)
    {
        MT_CORE_ERROR("Failed to load script module: {}", e.what());
        return py::module_();
    }
}
}  // namespace Matcha