local skynet = require "skynet"
local mysql = require "skynet.mysql"

local CMD = {}
local pool = {}
local idle_conns = {}  -- 空闲连接队列
local busy_conns = {}   -- 使用中连接
local max_conn = 10     -- 最大连接数
local min_conn = 2      -- 最小连接数

-- 初始化连接池
function CMD.init(config)
    for i = 1, min_conn do
        local conn = mysql.connect(config)
        table.insert(idle_conns, conn)
    end
end

-- 获取连接（带超时）
function CMD.get_conn(timeout)
    local start = skynet.now()
    while skynet.now() - start < timeout do
        if #idle_conns > 0 then
            local conn = table.remove(idle_conns, 1)
            busy_conns[conn] = true
            return conn
        elseif #busy_conns < max_conn then
            local conn = mysql.connect(config)
            busy_conns[conn] = true
            return conn
        end
        skynet.sleep(10)  -- 等待10ms重试
    end
    error("MySQL connection timeout")
end

-- 释放连接
function CMD.release_conn(conn)
    busy_conns[conn] = nil
    if #idle_conns < max_conn then
        table.insert(idle_conns, conn)
    else
        conn:disconnect()  -- 超限则关闭
    end
end

-- 执行SQL查询
function CMD.query(conn, sql, params)
    local ok, result = pcall(conn.query, conn, sql, params)
    if not ok then
        conn:disconnect()  -- 异常连接丢弃
        CMD.release_conn(conn)
        error(result)
    end
    return result
end

skynet.start(function()
    skynet.dispatch("lua", function(_, _, cmd, ...)
        local f = CMD[cmd]
        skynet.ret(skynet.pack(f(...)))
    end)
end)