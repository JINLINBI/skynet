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

local function call_mongodb_slave(addr, cmd, ...)
    return skynet.call(addr, "lua", cmd, ...)
end

local function send_mongodb_slave(addr, cmd, ...)
    skynet.send(addr, "lua", cmd, ...)
end

local function start()
    local setting = settings.db_cnf[skynet_node_name]
    INFO("mongodbpool 启动", skynet_node_name, inspect(setting))
    maxconn = tonumber(setting.mongodb_maxinst) or 1
    for i = 1, maxconn do
        local mongodb_slave = skynet.newservice("mongodb_slave")
        skynet.call(mongodb_slave, "lua", "start", setting.mongodb_cnf)
        table.insert(pool, mongodb_slave)
    end
end

function CMD.find(table_name, cname, ...)
    local executer = getconn()
    return call_mongodb_slave(executer, "find", table_name, cname, ...)
end

function CMD.findOne(table_name, cname, conds)
    local executer = getconn()
    return call_mongodb_slave(executer, "findOne", table_name, cname, conds)
end

function CMD.aggregate(dbname, cname, operation)
    local executer = getconn()
    return call_mongodb_slave(executer, "aggregate", table_name, cname, operation)
end

function CMD.upsert(table_name, cname, datas, conds)
    local executer = getconn()
    return call_mongodb_slave(executer, "update", table_name, cname, conds, datas)
end

function CMD.updateMany(table_name, cname, datas, conds)
    local executer = getconn()
    return call_mongodb_slave(executer, "updateMany", table_name, cname, conds, datas)
end

function CMD.insert(table_name, cname, datas)
    local executer = getconn()
    return call_mongodb_slave(executer, "insert", table_name, cname, datas)
end

function CMD.batch_insert(table_name, cname, datas)
    local executer = getconn()
    return call_mongodb_slave(executer, "batch_insert", table_name, cname, datas)
end

function CMD.del(table_name, cname, conds)
    local executer = getconn()
    return call_mongodb_slave(executer, "del", table_name, cname, conds)
end

skynet.start(function()
    start()

    skynet.dispatch("lua", function(_, _, cmd, ...)
        local f = assert(CMD[cmd], cmd .. " not found")
        skynet.retpack(f(...))
    end)

    skynet.register('.' .. SERVICE_NAME)
end)
