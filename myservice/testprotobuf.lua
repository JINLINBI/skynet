local skynet = require "skynet"
local protobuf = require "protobuf"
local inspect = require "inspect"



skynet.start(function()
    local targetFile = "proto/msg.proto"
    protobuf.load(targetFile)
    local id, msg = protobuf.encodeClt("Login", {
        account = "testuser",
        passwd = "123456",
        result = 10,
    })


    log_info("[testprotobuf main] encode msg", id, "type", type(msg), "data", msg)
    local sname, data = protobuf.decode(id, msg)
    log_info("[testprotobuf main] decode msg", sname, "data", inspect(data))

    skynet.fork(function ()
        while true do
            local id, msg = protobuf.encodeClt("Test", {
                account = "testuser",
                passwd = "123456",
                result = 10,
            })
            local sname, data = protobuf.decode(id, msg)
            log_info("[testprotobuf fork] decode msg", sname, "data", inspect(data))
            skynet.sleep(10 * 100)
        end
    end)
end)