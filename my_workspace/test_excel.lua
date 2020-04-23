local skynet = require("skynet")
local excel = require("excel")

skynet.start(function()
	local pieces_list = excel.pieces_list
	-- print("helloowlll:\n " .. item[2].name)
	local count = 1
	for i, excel_list_file in pairs(excel) do
		for i, line in pairs(excel_list_file) do
			print(">>>>>>>>>>>>>>>")
			if line.name then
				print("line's name: " .. line.name)
			end
			if line.text then
				print("line's name: " .. line.text)
			end
			local count = 1
			for key, value in pairs(line) do
				if type(value) ~= "table" then
					print(key .. ": " .. value)
				else
					for k, v in ipairs(value) do
						print("v: " .. v)
					end
					-- print("get table: " .. table.concat(value, ", "))
				end
				count = count + 1
				if count > 10 then
					break;
				end
			end
		end
	end
	
	-- local atb_data = excel.atb_data
	-- for i, line in pairs(atb_data) do
	-- 	print("********************")
	-- 	for key, value in pairs(line) do
	-- 		print(key .. ": " .. value)
	-- 	end
	-- end
	print("???????????????????")
	print(pieces_list[5].id)
	print(pieces_list[5].name)
end)
