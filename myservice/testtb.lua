local skynet = require "skynet"
local inspect = require "inspect"



skynet.start(function()

	skynet.fork(function()
		local confloader = require "confloader"
		local t = confloader.new("table/example.lua")
		if not t then return end

		while true do
			-- log_info(inspect(t:get("test", 1)))
			-- log_info(inspect(t:get("test2", 1)))
			log_info("test count", t:count("test"))
			log_info("test2 count", t:count("test2"))
			skynet.sleep(500)
		end
	end)
end)
