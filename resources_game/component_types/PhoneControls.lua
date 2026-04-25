PhoneControls = {
    speed = 5,
    jump_power = 500,
    
    HoldRight = function(self)
        if Input.GetFingerDown() and Input.GetFingerPosition().x > 650 then
            return true
        end
        return false
    end,
    
    HoldLeft = function(self)
        if Input.GetFingerDown() and Input.GetFingerPosition().x < 250 then
            return true
        end
        return false
    end,

    OnStart = function(self)
        self.rb = self.actor:GetComponent("Rigidbody")
    end,

    OnUpdate = function(self)
        if Input.GetLongPress() then
            Debug.Log(Input.GetFingerPosition().x)
            Debug.Log(Input.GetFingerPosition().y)
        end
        
        -- Horizontal
        local horizontal_input = 0
        if self:HoldRight() then
            horizontal_input = self.speed
        end

        if self:HoldLeft() then
            horizontal_input = -self.speed
        end

        -- Vertical
        local vertical_input = 0

        -- Check on ground
        local on_ground = false
        ground_object = Physics.Raycast(self.rb:GetPosition(), Vector2(0, 1), 1)
        if Input.GetSwipe("up") then
            if ground_object ~= nil then
                vertical_input = -self.jump_power
            end
        end

        self.rb:AddForce(Vector2(horizontal_input, vertical_input))
        
        if Input.GetDoubleTap() then
            self.rb:SetPosition(Vector2(2, 17))
        end
    end
}
