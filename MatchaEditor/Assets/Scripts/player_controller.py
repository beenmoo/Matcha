import matcha_engine as mt


class PlayerController:
    input: mt.Input
    entity: mt.Entity
    time: mt.Time
    
    def on_create(self):
        self.move_speed = 2.0

    def on_update(self):
        transform = self.entity.get_transform()

        movement = mt.Vector3(0.0)

        if self.input.get_key(mt.KeyCode.W):
            movement = movement + transform.get_forward()
        if self.input.get_key(mt.KeyCode.S):
            movement = movement - transform.get_forward()
        if self.input.get_key(mt.KeyCode.D):
            movement = movement + transform.get_right()
        if self.input.get_key(mt.KeyCode.A):
            movement = movement - transform.get_right()
        if self.input.get_key(mt.KeyCode.SPACE):
            movement = movement + mt.Vector3(0.0, 1.0, 0.0)
        if self.input.get_key(mt.KeyCode.LCTRL):
            movement = movement - mt.Vector3(0.0, 1.0, 0.0)

        if movement != mt.Vector3(0.0):
            delta_time = self.time.get_delta_time()
            transform.set_position(transform.get_position() + (mt.normalize(movement) * self.move_speed * delta_time))