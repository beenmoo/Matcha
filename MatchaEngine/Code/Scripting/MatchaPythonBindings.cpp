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
//
// Surface bound here is exactly what RotationComponent/Flashlight/CameraController (the three
// scripts this replaced NativeScriptComponent's compiled versions of) need - not the whole engine
// API speculatively. KeyCode in particular only exposes the handful of scancodes CameraController
// actually reads (A/S/D/W/SPACE/LCTRL out of ~200 in KeyCodes.h) - add more as real scripts need
// them.
#include "MatchaPythonBindings.h"
#include "Core/EngineContext.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/Time.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector.h"
#include "Scene/Component/LightComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include <pybind11/embed.h>
#include <pybind11/operators.h>

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

    py::class_<Vector2Int>(m, "Vector2Int")
        .def_readwrite("x", &Vector2Int::x)
        .def_readwrite("y", &Vector2Int::y);

    py::class_<Vector3>(m, "Vector3")
        .def(py::init<float>())
        .def(py::init<float, float, float>())
        .def_readwrite("x", &Vector3::x)
        .def_readwrite("y", &Vector3::y)
        .def_readwrite("z", &Vector3::z)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * float())
        .def(-py::self)
        .def(py::self == py::self);

    m.def("normalize", &Normalize);
    m.def("radians", static_cast<float (*)(float)>(&Radians));

    py::class_<Quaternion>(m, "Quaternion")
        .def(py::init<float, float, float, float>())
        .def(py::self * py::self)
        .def(
            "__mul__", [](const Quaternion& q, const Vector3& v) { return q * v; }, py::is_operator());

    m.def("angle_axis", &AngleAxis);

    py::class_<Transform> transform(m, "Transform");

    // Space has to be registered before rotate() below: its default argument value
    // (Transform::Space::Self) needs the enum type already known to pybind11 to convert it to a
    // Python object at binding-registration time - confirmed by an actual run, which failed with
    // "could not convert default argument ... (type not registered yet?)" when this was ordered
    // the other way around.
    py::enum_<Transform::Space>(transform, "Space")
        .value("WORLD", Transform::Space::World)
        .value("SELF", Transform::Space::Self);

    transform.def("get_position", &Transform::GetPosition)
        .def("set_position", static_cast<void (Transform::*)(const Vector3&)>(&Transform::SetPosition))
        .def("get_rotation", &Transform::GetRotation)
        .def("set_rotation", &Transform::SetRotation)
        .def("get_forward", &Transform::GetForward)
        .def("get_right", &Transform::GetRight)
        .def("translate", static_cast<void (Transform::*)(const Vector3&)>(&Transform::Translate))
        .def("rotate", static_cast<void (Transform::*)(const Vector3&, float, Transform::Space)>(&Transform::Rotate),
             py::arg("axis"), py::arg("angle_degrees"), py::arg("space") = Transform::Space::Self);

    py::enum_<LightType>(m, "LightType")
        .value("DIRECTIONAL", LightType::Directional)
        .value("POINT", LightType::Point)
        .value("SPOT", LightType::Spot);

    py::class_<LightComponent>(m, "LightComponent")
        .def_readwrite("type", &LightComponent::type)
        .def_readwrite("color", &LightComponent::color)
        .def_readwrite("intensity", &LightComponent::intensity)
        .def_readwrite("range", &LightComponent::range)
        .def_readwrite("inner_cone_angle", &LightComponent::innerConeAngle)
        .def_readwrite("outer_cone_angle", &LightComponent::outerConeAngle);

    py::class_<Entity>(m, "Entity")
        // reference, not reference_internal, throughout: components are owned by the Scene's
        // registry, not by the Entity handle (a lightweight (handle, Scene*) pair) - there's
        // nothing for pybind11's keep-alive machinery to tie the returned reference's lifetime to.
        .def(
            "get_transform",
            [](Entity& self) -> Transform& { return self.GetComponent<TransformComponent>().transform; },
            py::return_value_policy::reference)
        .def(
            "add_light_component",
            [](Entity& self) -> LightComponent& { return self.AddComponent<LightComponent>(); },
            py::return_value_policy::reference);

    py::class_<Scene>(m, "Scene")
        .def("create_entity", [](Scene& self) { return self.CreateEntity(); });

    py::enum_<KeyCode>(m, "KeyCode")
        .value("A", KeyCode::A)
        .value("S", KeyCode::S)
        .value("D", KeyCode::D)
        .value("W", KeyCode::W)
        .value("SPACE", KeyCode::SPACE)
        .value("LCTRL", KeyCode::LCTRL);

    py::enum_<Input::MouseButton>(m, "MouseButton")
        .value("LEFT", Input::MouseButton::Left)
        .value("MIDDLE", Input::MouseButton::Middle)
        .value("RIGHT", Input::MouseButton::Right)
        .value("BACK", Input::MouseButton::Back)
        .value("FORWARD", Input::MouseButton::Forward);

    py::enum_<Input::AxisType>(m, "AxisType")
        .value("MOUSE", Input::AxisType::Mouse)
        .value("JOYSTICK", Input::AxisType::Joystick);

    py::enum_<Input::CursorLockState>(m, "CursorLockState")
        .value("NONE", Input::CursorLockState::None)
        .value("LOCKED", Input::CursorLockState::Locked);

    py::class_<Input>(m, "Input")
        .def("get_key", &Input::GetKey)
        .def("get_mouse_button", &Input::GetMouseButton)
        .def("get_mouse_button_down", &Input::GetMouseButtonDown)
        .def("get_mouse_button_up", &Input::GetMouseButtonUp)
        .def("get_axis", &Input::GetAxis)
        .def("set_cursor_lock_state", &Input::SetCursorLockState);

    py::class_<Time>(m, "Time")
        .def("get_delta_time", &Time::GetDeltaTime);

    py::class_<EngineContext>(m, "EngineContext")
        .def(
            "get_input", [](EngineContext& self) -> Input& { return self.GetInput(); }, py::return_value_policy::reference)
        .def(
            "get_time", [](EngineContext& self) -> Time& { return self.GetTime(); }, py::return_value_policy::reference)
        .def(
            "get_scene", [](EngineContext& self) -> Scene& { return self.GetScene(); }, py::return_value_policy::reference);
}
