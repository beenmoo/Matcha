import matcha_engine

# Port of Sandbox/Code/Core/Flashlight. Creates its own light entity in on_create() and re-pins
# that light's transform to match its host every on_update(), so binding this alongside other
# scripts (e.g. camera_controller) works without either needing to know about the other.


class Flashlight:
    # See RotationComponent's own copy of this comment for why these are bare annotations.
    entity: matcha_engine.Entity
    scene: matcha_engine.Scene
    light: matcha_engine.Entity

    def on_create(self):
        scene = self.scene
        self.light = scene.create_entity()

        light = self.light.add_light_component()
        light.type = matcha_engine.LightType.SPOT
        light.color = matcha_engine.Vector3(0.6, 0.8, 1.0)
        light.intensity = 5.0
        light.range = 6.0
        light.inner_cone_angle = 12.5
        light.outer_cone_angle = 20.0
        
        self.light.get_transform().set_position(self.entity.get_transform().get_position())
        self.light.set_parent(self.entity)
