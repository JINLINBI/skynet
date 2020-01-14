local skynet = require("skynet")
local excel = require("excel")

skynet.start(function()
	local item = excel.item_list
	-- print("helloowlll:\n " .. item[2].name)
	local count = 1
	for i, line in pairs(item) do
		-- print("index: " .. i)
		-- if line then
		-- 	print("line's col count: " .. #line)
		-- 	print("line's id: " .. line.id)
		-- 	print("line's name: " .. line.name)
		-- 	print("line's type: " .. line.type)
		-- 	print("line's life: " .. line.life)
		-- 	for i, value in pairs(line) do
		-- 		print(value)
		-- 	end
		-- end
		print(">>>>>>>>>>>>>>>")
		for key, value in pairs(line) do
			print(key .. ": " .. value)
		end
	end
	-- print(item[1])
	print(item[5].name)
end)
