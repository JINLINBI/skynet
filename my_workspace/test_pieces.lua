local skynet = require("skynet")
local pieces = require("pieces")

function delay(p)
	skynet.sleep(300)
		print("p.born_time is :" .. os.date("%Y-%m-%d %H:%M:%S", p.born_time))
	print("now is :" .. os.date("%Y-%m-%d %H:%M:%S", os.time()))
end

skynet.start(function()
	local p = pieces.new()
	print("p.onlyId is :" .. p.only_id);
	print("p.dirty is :" .. p.dirty);
	print("p.copy is :" .. p.copy);
	print("p.data is :" .. p.data);
	print("p.born_time is :" .. p.born_time);
	print("build_time is :" .. p.build_time);
	print("now is .." .. os.time());
	print("p.born_time is :" .. os.date("%Y-%m-%d %H:%M:%S", p.born_time));
	print("now is .." .. os.date("%Y-%m-%d %H:%M:%S", os.time()));

	
	p.excel_id = 3
	print("pieces.excel.name: " .. p.excel.name)

	p.excel_id = 4                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
	print("pieces.excel.name: " .. p.excel.name)

	-- p.flagint32[pieces_flag32.helloword] = 343
	-- p.flagString[pieces_string.myname] = "hello world"
	-- local player = p
	-- player.flagatb[pieces_flagatb.attack] 
	-- player.flagobj[pieces_flagobj.team] = { team_name = "team_name", 
	-- 										leader_name = "leader_name", 
	-- 										teamate_count = 4}
	-- skynet.fork(delay, p)
	-- for i, value in pairs(p.onlyId) do
	-- 	if type(value) == "number" or type(value) == "string" then
	-- 		print("i: " .. i)
	-- 		print("value: " .. value)
	-- 	end
	-- end
	-- local pieces_data = p.onlyId["pieces_userdata"]
	-- print("get userdata: " .. pieces_data.Id)
	-- skynet.exit()
end)
