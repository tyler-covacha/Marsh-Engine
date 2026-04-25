
ImageUIRenderer = {
    sprite = "???",
    r = 255,
    g = 255,
    b = 255,
    a = 255,
    sorting_order = 999,

    OnStart = function(self)
        Audio.Play(5, "TitleScreen_1A_mp3", 0)
    end,

    OnUpdate = function(self)
        self.pos = Vector2(0, 0)
        self.rot_degrees = 0
        Camera.SetPosition(0,0)
        Image.DrawUIEx(self.sprite, 0,0,self.r,self.g,self.b,self.a,self.sorting_order, 0.5,0.4)
    end
}

