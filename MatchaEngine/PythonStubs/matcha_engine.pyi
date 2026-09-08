# Type stub for the `matcha_engine` module - not a real Python package, but the
# PYBIND11_EMBEDDED_MODULE registered by MatchaEngine/Code/Scripting/MatchaPythonBindings.cpp,
# which only exists inside the embedded interpreter at runtime. Pylance/Pyright has no way to
# discover that binding surface on its own, so this hand-written stub is what makes "import
# matcha_engine" resolve, gives every script's fields real types, and gets autocomplete working
# in RotationComponent/Flashlight/CameraController and any script written after them.
#
# Kept in sync by hand with MatchaPythonBindings.cpp - there's no generator wired up (pybind11-
# stubgen needs to import the real module from a standalone interpreter, which an embedded-only
# module was never built to support). When a .def()/.def_readwrite() is added there, add the
# matching declaration here.

from typing import overload

class Vector2Int:
    x: int
    y: int
    def __init__(self, x: int = ..., y: int = ...) -> None: ...

class Vector3:
    x: float
    y: float
    z: float
    @overload
    def __init__(self, scalar: float) -> None: ...
    @overload
    def __init__(self, x: float, y: float, z: float) -> None: ...
    def __add__(self, other: Vector3) -> Vector3: ...
    def __sub__(self, other: Vector3) -> Vector3: ...
    def __mul__(self, scalar: float) -> Vector3: ...
    def __neg__(self) -> Vector3: ...
    def __eq__(self, other: object) -> bool: ...

def normalize(v: Vector3) -> Vector3: ...
def radians(degrees: float) -> float: ...

class Quaternion:
    def __init__(self, x: float, y: float, z: float, w: float) -> None: ...
    @overload
    def __mul__(self, other: Quaternion) -> Quaternion: ...
    @overload
    def __mul__(self, v: Vector3) -> Vector3: ...

def angle_axis(angle_radians: float, axis: Vector3) -> Quaternion: ...

class Transform:
    class Space:
        WORLD: Transform.Space
        SELF: Transform.Space

    def get_position(self) -> Vector3: ...
    def set_position(self, position: Vector3) -> None: ...
    def get_rotation(self) -> Quaternion: ...
    def set_rotation(self, rotation: Quaternion) -> None: ...
    def get_forward(self) -> Vector3: ...
    def get_right(self) -> Vector3: ...
    def translate(self, delta: Vector3) -> None: ...
    def rotate(self, axis: Vector3, angle_degrees: float, space: Transform.Space = ...) -> None: ...

class LightType:
    DIRECTIONAL: LightType
    POINT: LightType
    SPOT: LightType

class LightComponent:
    type: LightType
    color: Vector3
    intensity: float
    range: float
    inner_cone_angle: float
    outer_cone_angle: float

class Entity:
    # is_valid() is what lets a script hold an Entity across frames (Flashlight caches the light
    # entity it spawns) without crashing if it's deleted out from under the script later - see
    # MatchaPythonBindings.cpp's RequireValidEntity for what every other accessor below does if
    # this isn't checked first.
    def is_valid(self) -> bool: ...
    def get_transform(self) -> Transform: ...
    def add_light_component(self) -> LightComponent: ...

class Scene:
    def create_entity(self) -> Entity: ...

class KeyCode:
    # Only the scancodes CameraController actually reads are bound - add more here as real
    # scripts need them, matching whatever gets added to the py::enum_<KeyCode> in
    # MatchaPythonBindings.cpp.
    A: KeyCode
    S: KeyCode
    D: KeyCode
    W: KeyCode
    SPACE: KeyCode
    LCTRL: KeyCode

class MouseButton:
    LEFT: MouseButton
    MIDDLE: MouseButton
    RIGHT: MouseButton
    BACK: MouseButton
    FORWARD: MouseButton

class AxisType:
    MOUSE: AxisType
    JOYSTICK: AxisType

class CursorLockState:
    NONE: CursorLockState
    LOCKED: CursorLockState

class Input:
    def get_key(self, code: KeyCode) -> bool: ...
    def get_mouse_button(self, button: MouseButton) -> bool: ...
    def get_mouse_button_down(self, button: MouseButton) -> bool: ...
    def get_mouse_button_up(self, button: MouseButton) -> bool: ...
    def get_axis(self, axis_type: AxisType) -> Vector2Int: ...
    def set_cursor_lock_state(self, state: CursorLockState) -> None: ...

class Time:
    def get_delta_time(self) -> float: ...
