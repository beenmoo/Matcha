#pragma once

// See the identical guard in Scene/Component/PythonScriptComponent.h: Qt's <QObject> #defines
// "slots" to nothing, which mangles CPython's own "PyType_Slot *slots;" struct member into a
// syntax error. This header isn't currently included from anywhere Qt-based, but the collision
// depends on include order in whatever consumer eventually does - guarding it here once means no
// future consumer has to rediscover this the hard way.
#pragma push_macro("slots")
#undef slots
#include <pybind11/embed.h>
#pragma pop_macro("slots")

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