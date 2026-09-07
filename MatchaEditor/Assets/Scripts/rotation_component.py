import matcha_engine

# Port of Sandbox/Code/Core/RotationComponent (native C++ script, removed when Python scripting
# replaced NativeScriptComponent - see MatchaPythonBindings.cpp for what's exposed).


class RotationComponent:
    # Set by PythonScriptSystem right after construction, before on_create() runs - not
    # assigned in __init__ (nothing here to initialize them to yet), so they're declared as
    # bare annotations instead: enough for Pylance to type self.entity/self.context, with no
    # runtime effect (an annotation with no value adds nothing to __dict__ or __init__).
    entity: matcha_engine.Entity
    context: matcha_engine.EngineContext

    def __init__(self):
        self.degrees_per_second = 45.0

    def on_update(self):
        delta_time = self.context.get_time().get_delta_time()

        transform = self.entity.get_transform()
        transform.rotate(matcha_engine.Vector3(0.0, 1.0, 0.0), self.degrees_per_second * delta_time)
