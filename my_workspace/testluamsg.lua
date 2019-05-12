skynet = require "skynet"
require "skynet.manager"


local function dosomething(session, address, ...)
    skynet.error("session", session)
    skynet.error("address", skynet.address(address))
    local args = {...}

    for i, v in pairs(args) do
        skynet.error("arg" .. i .. ":", v)
    end
end


skynet.start(function()
    skynet.dispatch("lua", function(session, address, ...)
        dosomething(session, address, ...)
    end)
    skynet.register(".testluamsg")
end)
