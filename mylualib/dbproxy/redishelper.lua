local M = {}

local redisdbx = require "dbproxy.redisdbx"


function M.exec(cmd, uid, ...)
    return redisdbx.exec(cmd, uid, ...)
end
return M
