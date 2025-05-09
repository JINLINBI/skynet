local skynet = require "skynet"
local playerdata = require "playerdata"
local inspect = require "inspect"
local lu = require "luaunit"


skynet.start(function()
    local player = playerdata.new()
    player:add_listener(function(path, action, key, value)
        log_info("path", path, "action", action, "key", key, "value", value)--, debug.traceback())
    end)

    local testtb = { helloworld = {} }
    log_info("get version", player:getVersion())
    table.insert(player, testtb)

    log_info(testtb, player[1])
    log_info("get version", player:getVersion())
    log_info("player[1].helloworld =", player[1].helloworld)
    log_info("get version", player:getVersion())
    table.insert(player[1].helloworld, "world2")
    log_info("get version", player:getVersion())
    table.insert(player[1].helloworld, "world1")

    log_info("get version", player:getVersion())
    player[1].helloworld["mapidx"] = "mapvalue"
    log_info("get version", player:getVersion())

    for k, v in pairs(player[1].helloworld) do
        log_info("pairs k", k, "v", v)
    end

    for k, v in ipairs(player[1].helloworld) do
        log_info("ipairs k", k, "v", v)
    end

    log_info("get version", player:getVersion())
    -- log_info("inspect player\n", inspect.inspect(player))

    table.remove(player[1].helloworld)
    table.remove(player[1].helloworld)
    table.remove(player[1].helloworld)
    player[1].helloworld = nil
    player[1].helloworld = nil
    player[1] = nil

    log_info("get version", player:getVersion())
    -- testtb.helloworld = nil
    -- testtb.helloworld = 2
end)
