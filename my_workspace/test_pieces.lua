local skynet = require("skynet")
local pieces = require("pieces")

function delay(p)
	skynet.sleep(300)
		print("p.born_time is: " .. os.date("%Y-%m-%d %H:%M:%S", p.born_time))
	print("now is: " .. os.date("%Y-%m-%d %H:%M:%S", os.time()))
end

skynet.start(function()
	local p = pieces.new()
	print("p.onlyId is: " .. p.only_id);
	print("p.is_dirty is: " .. p.is_dirty);
	print("p.is_copy is: " .. p.is_copy);
	print("p.is_data is: " .. p.is_data);
	print("p.born_time is: " .. p.born_time);
	print("build_time is: " .. p.build_time);
	print("now is .." .. os.time());
	print("p.born_time is: " .. os.date("%Y-%m-%d %H:%M:%S", p.born_time));
	print("now is .." .. os.date("%Y-%m-%d %H:%M:%S", os.time()));

	p.excel_id = 4                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
	print("p.excel.name: " .. p.excel.name)

	p.excel.name = "helloworld"
	print("p.excel.name = \"helloworld\"")
	print("p.excel.name: " .. p.excel.name)
	print("p.__excel.name: " .. p.__excel.name)
	print("p.excel.name: " .. p.excel.name)
	print("p.__excel.name: " .. p.__excel.name)
	for i, v in pairs(p.excel.career) do
		print("career: " .. v)
	end
	p.excel.career = {234, 23444, 234342, 32423, "23432", "diealone,,."}
	p.excel.name = nil
	print("p.excel.career = {234, 23444, 234342, 32423, \"23432\", \"diealone,,.\"}")
	for i, v in pairs(p.excel.career) do
		print("career: " .. v .. " type: " .. type(v))
	end

	for i, v in pairs(p.excel.log) do
		print("log: " .. v)
	end

	p.excel.log = {"hello", "world", 3434}
	print("p.excel.log = {\"hello\", \"world\", 3434}")
	for i, v in pairs(p.excel.log) do
		print("log: " .. v .. " type: " .. type(v))
	end
	print("p.excel.name = nil")
	print("p.excel.name: " .. p.excel.name)
	print("p.__excel.name: " .. p.__excel.name)
	print("p.is_excel: " .. p.is_excel)
end)
