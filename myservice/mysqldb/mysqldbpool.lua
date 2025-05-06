local skynet = require "skynet.manager"
local settings = require "settings"
local inspect = require "inspect"

local skynet_node_name = ...

local CMD = {}
local pool = {}

local next_id = 0
local maxconn = 1

local function next_conn()
    local id = next_id % maxconn + 1
    next_id = next_id + 1
    if id > maxconn then
        id = 1
    end
    return pool[id]
end

local function getconn(key)
    if key and (type(key) == "number" or tonumber(key)) then
        local id = math.floor((tonumber(key) - 1) % maxconn) + 1
        return pool[id]
    else
        return next_conn()
    end
end

local function call_mysqldb_slave(addr, ...)
    return skynet.call(addr, "lua", "query", ...)
end

local function call_mysqldb_slave_prepare(addr, ...)
    return skynet.call(addr, "lua", "prepare", ...)
end

local function call_mysqldb_slave_exec(addr, ...)
    return skynet.call(addr, "lua", "execute", ...)
end

local function call_mysqldb_slave_execstmt(addr, ...)
    return skynet.call(addr, "lua", "executestmt", ...)
end

local function start()
    local setting = settings.nodes[skynet_node_name]
    maxconn = tonumber(setting.mysqldb_maxinst) or 1
    INFO("mysqldb pool 启动", skynet_node_name, "maxconn", maxconn) --, inspect(setting))
    for _ = 1, maxconn do
        local mysqldb_slave = skynet.newservice("mysqldb_slave")
        skynet.call(mysqldb_slave, "lua", "start", setting.mysqldb_cnf)
        table.insert(pool, mysqldb_slave)
    end
end


function CMD.query(sql)
    local executer = getconn()
    return call_mysqldb_slave(executer, sql)
end


function CMD.prepare(prepare, ...)
    local executer = getconn()
    return call_mysqldb_slave_prepare(executer, prepare, ...)
end

function CMD.execute(prepare, ...)
    local executer = getconn()
    return call_mysqldb_slave_exec(executer, prepare, ...)
end

function CMD.executestmt(stmt, ...)
    local executer = getconn()
    return call_mysqldb_slave_execstmt(executer, stmt, ...)
end

skynet.start(function()
    start()

    skynet.dispatch("lua", function(_, _, cmd, ...)
        local f = assert(CMD[cmd], cmd .. "not found")
        skynet.retpack(f(...))
    end)

    skynet.register('.' .. SERVICE_NAME)
end)
