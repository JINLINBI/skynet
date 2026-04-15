local skynet = require "skynet"
local mysql = require "skynet.mysql"

local CMD = {}
local pool = {}
local idle_conns = {}  -- 空闲连接队列
local busy_conns = {}   -- 使用中连接
local max_conn = 10     -- 最大连接数
local min_conn = 2      -- 最小连接数
local config = nil      -- 存储配置

-- 初始化连接池
function CMD.init(cfg)
    config = cfg
    for i = 1, min_conn do
        local conn = mysql.connect(config)
        if conn then
            table.insert(idle_conns, conn)
        else
            skynet.error("Failed to create initial connection", i)
        end
    end
end

-- 获取连接（带超时）
function CMD.get_conn(timeout)
    timeout = timeout or 3000  -- 默认3秒超时
    local start = skynet.now()
    while skynet.now() - start < timeout do
        if #idle_conns > 0 then
            local conn = table.remove(idle_conns, 1)
            busy_conns[conn] = true
            return conn
        elseif #busy_conns + #idle_conns < max_conn then
            local conn = mysql.connect(config)
            if conn then
                busy_conns[conn] = true
                return conn
            else
                skynet.error("Failed to create new connection")
            end
        end
        skynet.sleep(10)  -- 等待10ms重试
    end
    error("MySQL connection timeout after " .. timeout .. "ms")
end

-- 释放连接
function CMD.release_conn(conn)
    if not conn then
        return
    end

    busy_conns[conn] = nil
    if #idle_conns < max_conn then
        table.insert(idle_conns, conn)
    else
        conn:disconnect()  -- 超限则关闭
    end
end

-- 执行SQL查询（带自动连接管理）
function CMD.query(sql, params)
    local conn = CMD.get_conn(3000)
    local ok, result = pcall(conn.query, conn, sql, params)
    if not ok then
        skynet.error("MySQL query failed:", result)
        busy_conns[conn] = nil
        conn:disconnect()  -- 异常连接丢弃
        error(result)
    end
    CMD.release_conn(conn)
    return result
end

-- 关闭所有连接
function CMD.close_all()
    for _, conn in ipairs(idle_conns) do
        conn:disconnect()
    end
    for conn, _ in pairs(busy_conns) do
        conn:disconnect()
    end
    idle_conns = {}
    busy_conns = {}
end

-- 获取连接池状态
function CMD.status()
    local busy_count = 0
    for _ in pairs(busy_conns) do
        busy_count = busy_count + 1
    end
    return {
        idle = #idle_conns,
        busy = busy_count,
        total = #idle_conns + busy_count,
        max = max_conn,
    }
end

skynet.start(function()
    skynet.dispatch("lua", function(_, _, cmd, ...)
        local f = CMD[cmd]
        if not f then
            skynet.error("Unknown command:", cmd)
            return skynet.ret(skynet.pack(nil, "Unknown command"))
        end
        local ok, result = pcall(f, ...)
        if ok then
            skynet.ret(skynet.pack(result))
        else
            skynet.error("Command failed:", cmd, result)
            skynet.ret(skynet.pack(nil, result))
        end
    end)
end)