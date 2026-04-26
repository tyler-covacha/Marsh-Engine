
ControlsText = {
    sprite = "???",
    r = 255,
    g = 255,
    b = 255,
    a = 255,
    sorting_order = 999,

    OnStart = function(self)
        Audio.Play(5, "Battle_1B", 0)
    end,

    OnUpdate = function(self)
        self.pos = Vector2(0, 0)
        self.rot_degrees = 0
        MoveText = "Hold right side of screen to move right, left side to move left." 
        JumpText = "Tap to jump."
        ResetText = "Double tap to reset position."
        font = "NotoSans-Regular"
        font_size = 24
        Text.Draw(MoveText, 100, 100, font, font_size, self.r, self.g, self.b, self.a)
        Text.Draw(JumpText, 100, 150, font, font_size, self.r, self.g, self.b, self.a)
        Text.Draw(ResetText, 100, 200, font, font_size, self.r, self.g, self.b, self.a)
    end
}
