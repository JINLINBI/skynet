local skynet = require "skynet"


skynet.start(function()
    local gate = skynet.newservice("simplemsgserver")
    skynet.call(gate, "lua", "open", {
        port = 8002,
        maxclient = 64,
        servername = "sample",
    })
end)
