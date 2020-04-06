local skynet = require("skynet")
local pieces = require("pieces")


skynet.start(function()
	local pi = pieces.new()
	-- print("onlyId : " .. pi.onlyId);
	print("pi type: " .. type(pi))
	pi.excelId = 23423423
	pi.save_func = function()
		print("after save")
	end
	pi:save()
end)
