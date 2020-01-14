local skynet = require("skynet")
local excel = require("excel")

skynet.start(function()
	local item_list = excel.item_list
	-- print("helloowlll:\n " .. item[2].name)
	local count = 1
	for i, line in pairs(item_list) do
		print(">>>>>>>>>>>>>>>")
		print("line's name: " .. line.name)
		for key, value in pairs(line) do
			print(key .. ": " .. value)
		end
	end
	print("???????????????????")
	print(item_list[5].id)
	print(item_list[5].name)
end)
