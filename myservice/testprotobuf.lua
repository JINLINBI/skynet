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


    print("encode msg", id, "type", type(msg), "data", msg)
    local sname, data = protobuf.decode(id, msg)
    print("decode msg", sname, "data", inspect(data))
end)