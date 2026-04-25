VictoryBox = {

	ShowWin = false,

	OnUpdate = function(self)
		if self.ShowWin then
			Text.Draw("You Win!", 200, 200, "NotoSans-Regular", 24, 255, 255, 255, 255)
		end
	end,

	OnTriggerEnter = function(self, collision)
		self.ShowWin = true
	end
}

