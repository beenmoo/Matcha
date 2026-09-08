#include "PythonRuntime.h"

#include "Core/Logger.h"
#include "MatchaPythonBindings.h"

namespace Matcha
{
namespace
{
// CPython locates its own standard library relative to the running executable, which works for
// a real python.exe install but not for an executable that merely embeds the interpreter (this
// engine's own tools live nowhere near vcpkg's python3 install) - left unset, Py_Initialize()
// can't find the stdlib and prints "Could not find platform independent libraries <prefix>" at
// startup, and any script that imports a real (non-frozen) stdlib module fails at runtime.
// PyConfig.home must be set before Py_InitializeFromConfig() runs, i.e. before m_Interpreter's
// own constructor - not something that can happen in PythonRuntime's constructor body, so this
// builds the whole scoped_interpreter (which pybind11 supports initializing from a PyConfig*)
// as the member's initializer expression instead.
py::scoped_interpreter CreateInterpreter()
{
#ifdef MATCHA_PYTHON_HOME
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    wchar_t* home = Py_DecodeLocale(MATCHA_PYTHON_HOME, nullptr);
    PyStatus status = PyConfig_SetString(&config, &config.home, home);
    PyMem_RawFree(home);

    if (PyStatus_Exception(status) != 0)
    {
        PyConfig_Clear(&config);
        MT_CORE_ERROR("Failed to configure embedded Python's home directory - falling back to autodetection");
        return py::scoped_interpreter();
    }

    return py::scoped_interpreter(&config);
#else
    return py::scoped_interpreter();
#endif
}
}  // namespace

PythonRuntime::PythonRuntime()
    : m_Interpreter(CreateInterpreter())
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