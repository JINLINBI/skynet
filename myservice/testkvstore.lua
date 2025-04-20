local skynet = require "skynet"
local kv = require "kvstore"


skynet.start(function ()
    log_info("[testkvstore] get helloworld", kv.get("helloworld"))
    log_info("[testkvstore] update helloworld", kv.set("helloworld", "1"))
    log_info("[testkvstore] update helloworld", kv.get("helloworld"))

    kv.set("count", "0")
    skynet.fork(function ()
        log_info("[testkvstore] get count start", skynet.time())
        for i = 1, 30000000 do
            kv.get("count")
        end

        log_info("[testkvstore] get count end", skynet.time())
        log_info("[testkvstore] get count", kv.get("count"))
    end)

    -- skynet.fork(function ()
    --     for i = 1, 1000000 do
    --         kv.set("count", tostring(tonumber(kv.get("count") + 1)))
    --     end

    --     log_info("[testkvstore] get count", kv.get("count"))
    -- end)

end)