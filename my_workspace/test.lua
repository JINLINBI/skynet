local skynet = require("skynet")
local excel = require("excel")

skynet.start(function()
    skynet.error(" test servcice start...")
    --    local a = excel.atb_data[1]
    print("type is : " .. type(excel))
    if type(excel) == "string" then
        print("string type: " .. excel)
    end
    for i, k in pairs(excel) do
        print("i is " .. i .. " value : " .. k[0])
    end
end)
