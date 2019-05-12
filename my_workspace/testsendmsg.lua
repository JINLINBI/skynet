skynet = require "skynet"
require "skynet.manager"

skynet.start(function()
    skynet.register(".testsendmsg")
    local testluamsg = skynet.localname(".testluamsg")
    local r = skynet.send(testluamsg, "lua", 1, "nengzhong", true)
    skynet.error("skynet.send return value:", r)


    r = skynet.rawsend(testluamsg, "lua", skynet.pack(2, "ennzohgnw", false))
    skynet.error("skynet.rawsend return value:", r)
end)
