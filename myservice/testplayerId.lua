local skynet = require "skynet"
local playerId = require "playerId"

skynet.start(function()
    -- log_info("[testplayerId] start", skynet.time())
    -- for i = 1, 25000000 do
    --     local id = playerId.next_id()
    -- end
    -- log_info("[testplayerId] end", skynet.time())

    local id = playerId.nextId()
    log_info("[testplayerId] 生成玩家playerId", id)
    log_info("[testplayerId] 生成玩家playerId短字符", playerId.id2ShortStr(id))
    log_info("[testplayerId] 生成玩家playerId短字符反", playerId.shortStr2Id(playerId.id2ShortStr(id)))
    local info = playerId.parseId(id)
    log_info("[testplayerId] 机器id", info.machine)
    log_info("[testplayerId] 时间戳", info.timestamp)
    log_info("[testplayerId] 序列号", info.sequence)
end)