
SceneManager = {
    next_scene_name = "???",

    OnUpdate = function(self)
        if Input.GetTap() then
            Scene.Load(self.next_scene_name)
        end
    end
}

