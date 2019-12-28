local skynet = require("skynet")
local pieces = require("pieces")


skynet.start(function()
    local p = pieces.new()
    print("p.onlyId is .." .. p.onlyId);
end)
