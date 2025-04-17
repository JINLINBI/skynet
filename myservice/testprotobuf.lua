local skynet = require "skynet"
local protobuf = require "pb"
local pb = require "mypb"
local inspect = require "inspect"
require "skynet.manager"	-- import skynet.register



skynet.start(function()
    local targetFile = "proto/msg.proto"
    local targetFile2 = "proto/msg2.proto"

    pb.LoadProtoFile(targetFile)
    pb.LoadProtoFile(targetFile2)
    local msgId, msgData = pb.PbEncodeClt("Login", {
        account = "testuser",
        passwd = "123456",
        result = 10,
    })

    -- for k, v in protobuf.types() do
    --     print("k", k, "v", v, "type", type(v))
    --     for k, v in protobuf.fields(v) do
    --         print(k, v)
    --     end
    -- end


    print("encode msg", msgId, "type", type(msgData), "data", msgData)
    local msgPre, msg = pb.PbDecode(msgId, msgData)

    print("decode msg", msgPre, "data", inspect(msg))
end)