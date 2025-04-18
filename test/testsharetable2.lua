local skynet = require "skynet"
local sharetable = require "skynet.sharetable"

local inspect = require "inspect"



skynet.start(function()
	local confloader = require "confloader"

	local t = confloader.new("table/example.lua")
	if t then
		print(inspect(t:get("test", 1)))
		print(inspect(t:get("test2", 1)))
		-- print("test count", t:count("test"))
		-- print(inspect(t:getByIndex("test", "test1_test3", "1001_鲤鱼")))
	else
		print("table/example empty")
	end
	-- print(inspect(myconfig))
	-- queryall_test()
end)
