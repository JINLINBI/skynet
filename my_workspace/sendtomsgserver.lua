local skynet = require "skynet"


skynet.start(function()
    local gate = skynet.newservice("simplemsgserver")
    skynet.call(gate, "lua", "open", {
        port = 8002,
        maxclient = 64,
        servername = "sample",
    })

    local uid = "jinlin"
    local secret = "1111111"
    local subid = skynet.call(gate, "lua", "login", uid, secret)
    skynet.error("lua login subid: ", subid)
    skynet.call(gate, "lua", "kick", uid, subid)
    skynet.call(gate, "lua", "close")
end)
