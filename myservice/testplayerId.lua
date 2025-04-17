local skynet = require "skynet"
local playerId = require "playerId"

skynet.start(function()
    -- print("start", skynet.time())
    -- for i = 1, 25000000 do
    --     local id = playerId.next_id()
    -- end
    -- print("end", skynet.time())

    local id = playerId.nextId()
    print("生成玩家playerId", id)
    print("生成玩家playerId短字符", playerId.id2ShortStr(id))
    print("生成玩家playerId短字符反", playerId.shortStr2Id(playerId.id2ShortStr(id)))
    local info = playerId.parseId(id)
    print("机器id", info.machine)
    print("时间戳", info.timestamp)
    print("序列号", info.sequence)
end)