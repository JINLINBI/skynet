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

local function call_redis_slave(addr, cmd, ...)
    return skynet.call(addr, "lua", cmd, ...)
end

local function send_redis_slave(addr, cmd, ...)
    skynet.send(addr, "lua", cmd, ...)
end

local function start()
    local setting = settings.nodes[skynet_node_name]
    maxconn = tonumber(setting.redisdb_maxinst) or 1
    INFO("redisdb pool 启动", skynet_node_name, "max conn", maxconn) --, setting.redisdb_cnf)
    for _ = 1, maxconn do
        local redis_slave = skynet.newservice("redisdb_slave")
        skynet.call(redis_slave, "lua", "start", setting.redisdb_cnf)
        table.insert(pool, redis_slave)
    end
end

function CMD.exec(cmd, uid, ...)
    local db = getconn(uid)
    return call_redis_slave(db, cmd, ...)
end

skynet.start(function()
    start()

    skynet.dispatch("lua", function(_, _, cmd, ...)
        local f = assert(CMD[cmd], cmd .. "not found")
        skynet.retpack(f(...))
    end)

    skynet.register("." .. SERVICE_NAME)
end)
