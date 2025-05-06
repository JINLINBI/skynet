local redisdbx = {}

local skynet = require "skynet"

local REDISABL_POOL


local function block_query()
    if not REDISABL_POOL then
        REDISABL_POOL = skynet.queryservice("redisdbpool")
    end
end

function redisdbx.exec(cmd, uid, ...)
    block_query()
    return skynet.call(REDISABL_POOL, "lua", "exec", cmd, uid, ...)
end

return redisdbx
