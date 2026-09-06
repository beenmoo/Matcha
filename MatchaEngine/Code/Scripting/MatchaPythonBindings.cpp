// PYBIND11_EMBEDDED_MODULE registers a static initializer that makes "import matcha_engine" work
// from any script - but MatchaEngine links as a static .lib, and a .obj containing nothing but a
// static initializer (no symbol anything else references) is exactly what a linker is free to
// strip from a static library. Confirmed by an actual run: without ForceLink() below, scripts
// failed with "ModuleNotFoundError: No module named 'matcha_engine'" despite everything compiling
// cleanly - the registration code was silently never linked in. ForceLink() gives PythonRuntime's
// constructor a real symbol to call, which forces this whole translation unit (and therefore its
// static initializer) into the final binary.
//
// One entry per exposed type, same convention as ComponentRegistry.cpp's per-component-type
// table for serialization - both exist so adding engine-facing surface means adding one entry
// here, not scattering pybind11 calls across the codebase.
#include "MatchaPythonBindings.h"
#include "Math/Transform.h"
#include "Math/Vector.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include <pybind11/embed.h>

namespace py = pybind11;

namespace Matcha
{
void ForceLinkMatchaPythonBindings()
{
}
}  // namespace Matcha

PYBIND11_EMBEDDED_MODULE(matcha_engine, m)
{
    using namespace Matcha;

    py::class_<Vector3>(m, "Vector3")
        .def(py::init<float, float, float>())
        .def_readwrite("x", &Vector3::x)
        .def_readwrite("y", &Vector3::y)
        .def_readwrite("z", &Vector3::z);

    py::class_<Transform>(m, "Transform")
        .def("get_position", &Transform::GetPosition)
        .def("set_position", py::overload_cast<const Vector3&>(&Transform::SetPosition));

    py::class_<Entity>(m, "Entity")
        // reference, not reference_internal: the Transform is owned by the Scene's registry, not
        // by the Entity handle (a lightweight (handle, Scene*) pair) - there's nothing for
        // pybind11's keep-alive machinery to tie the returned reference's lifetime to.
        .def(
            "get_transform",
            [](Entity& self) -> Transform& { return self.GetComponent<TransformComponent>().transform; },
            py::return_value_policy::reference);
}
