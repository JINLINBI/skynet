local skynet = require("skynet")
local pieces = require("pieces")


skynet.start(function()
    local p = pieces.new()
    print("p.onlyId is .." .. p.onlyId);
    -- for i, value in pairs(p.onlyId) do
    --     if type(value) == "number" or type(value) == "string" then
    --     	print("i: " .. i)
    --     	print("value: " .. value)
    --     end
    -- end
    -- local pieces_data = p.onlyId["pieces_userdata"]
    -- print("get userdata: " .. pieces_data.Id)
end)
