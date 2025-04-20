local skynet = require "skynet"

skynet.start(function()
    -- print("start", skynet.time())

    -- for i = 1, 25000000 do
    --     local id = playerId.next_id()
    -- end
    -- print("end", skynet.time())
    skynet.fork(function ()
        while true do
            skynet.sleep(1 *  100)
            skynet.fork(function ()
                local test = import("testhotupdate")
                test.test(skynet.time())
            end)
        end
    end)

end)