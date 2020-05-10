local skynet = require("skynet")
local excel_list = require("excel")

skynet.start(function(_, source, ...)
    skynet.error(" test servcice start...")
    --    local a = excel.atb_data[1]
    print("type is : " .. type(excel_list))
    for i, excel in pairs(excel_list) do
        print("i is " .. i .. " value : " .. excel[1].name)
    end
end)
