import matcha_engine

# Port of Sandbox/Code/Core/Flashlight. Creates its own light entity in on_create() and re-pins
# that light's transform to match its host every on_update(), so binding this alongside other
# scripts (e.g. camera_controller) works without either needing to know about the other.


class Flashlight:
    def on_create(self):
        scene = self.context.get_scene()
        self.light = scene.create_entity()

        light = self.light.add_light_component()
        light.type = matcha_engine.LightType.SPOT
        light.color = matcha_engine.Vector3(0.6, 0.8, 1.0)
        light.intensity = 5.0
        light.range = 6.0
        light.inner_cone_angle = 12.5
        light.outer_cone_angle = 20.0

    def on_update(self):
        host_transform = self.entity.get_transform()
        light_transform = self.light.get_transform()

        light_transform.set_position(host_transform.get_position())
        light_transform.set_rotation(host_transform.get_rotation())
