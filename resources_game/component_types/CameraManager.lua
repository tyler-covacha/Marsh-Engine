CameraManager = {
    zoom_factor = 0.1,
    desired_zoom_factor = 1.0,
    x = 0.0,
    y = 0.0,
    ease_factor = 0.2,
    offset_y = -1,
    pinch_prev_frame = false,
    init_desired_zoom_factor = 1.0,
    
    -- Rotation
    rotation = 0.0,
    desired_rotation = 0.0,
    rotation_prev_frame = false,
    init_desired_rotation = 0.0,

    OnStart = function(self)
        Camera.SetZoom(self.zoom_factor)
    end,

    OnLateUpdate = function(self)
        local player = Actor.Find("Player")
        if player == nil then
            return
        end

        -- Desired zoom factor is determined by pinch
        if Input.IsPinching() then
            if self.pinch_prev_frame == false then
                self.pinch_prev_frame = true
                self.init_desired_zoom_factor = self.desired_zoom_factor
            end
            local raw_scale = Input.GetPinchScale()
            
            -- Dampen the sensitivity
            local sensitivity = 0.3
            local dampened_scale = 1.0 + (raw_scale - 1.0) * sensitivity
            
            self.desired_zoom_factor = self.init_desired_zoom_factor * dampened_scale
        else
            self.pinch_prev_frame = false
        end
        
        -- Clamp zoom
        self.desired_zoom_factor = math.min(math.max(0.1, self.desired_zoom_factor), 2)

        -- Ease zoom
        self.zoom_factor = self.zoom_factor + (self.desired_zoom_factor - self.zoom_factor) * 0.1

        Camera.SetZoom(self.zoom_factor)

        -- Desired rotation is determined by rotation gesture
        if Input.IsRotating() then
            if self.rotation_prev_frame == false then
                self.rotation_prev_frame = true
                self.init_desired_rotation = self.desired_rotation
            end
            
            -- Get rotation angle and dampen it
            local raw_rotation = Input.GetRotationAngle()
            local rotation_sensitivity = 0.5
            local rotation_change = raw_rotation * rotation_sensitivity
            
            self.desired_rotation = self.init_desired_rotation + rotation_change
        else
            self.rotation_prev_frame = false
        end

        -- Ease rotation
        self.rotation = self.rotation + (self.desired_rotation - self.rotation) * 0.1

        Camera.SetRotation(self.rotation)

        -- Desired camera position is the player's position (with an offset)
        local player_transform = player:GetComponent("Transform")
        local desired_x = player_transform.x
        local desired_y = player_transform.y + self.offset_y

        -- Mouse influences camera position
        local mouse_pos = Input.GetMousePosition()
        desired_x = desired_x + (mouse_pos.x - 576) * 0.01
        desired_y = desired_y + (mouse_pos.y - 256) * 0.01

        -- Ease position
        self.x = self.x + (desired_x - self.x) * self.ease_factor
        self.y = self.y + (desired_y - self.y) * self.ease_factor

        Camera.SetPosition(self.x, self.y)
    end
}
