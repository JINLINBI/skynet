local skynet = require("skynet")
local pieces = require("pieces")


skynet.start(function()
	local root = pieces.root()
	print("root born_time: " .. os.date("%y-%m-%d %H:%M:%S", root.born_time))
end)
