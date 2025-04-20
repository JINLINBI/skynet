local skynet = require "skynet"
local confloader = require "confloader"

skynet.start(function()
    -- print("start", skynet.time())

    -- for i = 1, 25000000 do
    --     local id = playerId.next_id()
    -- end
    -- print("end", skynet.time())
    skynet.fork(function ()
        local t = confloader.new("table/example.lua", function (t)
            log_info("[testconfloader] table update test???????????")
        end)
        -- if not t then return end
        log_info("[testconfloader] t", t)

        while true do
            log_info("[testconfloader] testconfig count", t:count("test"))
            skynet.sleep(5 * 100)
        end
    end)

end)