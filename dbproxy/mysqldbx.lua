local mysqldbx = {}

local skynet = require "skynet"

local MYSQLABL_POOL

-- 有些 服务 不允许在 init 阶段处理
-- 暂时需改成 第一次调用 查询
-- skynet.init(function ()
--     MYSQLABL_POOL = skynet.queryservice("mysqldbpool")
-- end)


local function block_query()
    -- body
    if not MYSQLABL_POOL then
        MYSQLABL_POOL = skynet.queryservice("mysqldbpool")
    end
end

function mysqldbx.query(sql)
    block_query()
    return skynet.call(MYSQLABL_POOL, "lua", "query", sql)
end

function mysqldbx.prepare(prepare, ...)
    block_query()
    return skynet.call(MYSQLABL_POOL, "lua", "prepare", prepare, ...)
end

function mysqldbx.execute(prepare, ...)
    block_query()
    return skynet.call(MYSQLABL_POOL, "lua", "execute", prepare, ...)
end

function mysqldbx.executestmt(stmt, ...)
    block_query()
    return skynet.call(MYSQLABL_POOL, "lua", "executestmt", stmt, ...)
end

return mysqldbx
