local skynet = require "skynet"
local playerdata = require "playerdata"
local inspect = require "inspect"
local pretty = require "pl.pretty"


skynet.start(function()
    local player = playerdata.new()
    player:add_listener(function(path, action, key, value)
        log_info("path", path, "action", action, "key", key, "value", value)
    end)

    local testtb = { helloworld = {} }
    table.insert(player, testtb)

    log_info("player[1].helloworld =", player[1].helloworld)
    table.insert(player[1].helloworld, "world2")
    table.insert(player[1].helloworld, "world1")

    player[1].helloworld["mapidx"] = "mapvalue"

    for k, v in pairs(player[1].helloworld) do
        log_info("pairs k", k, "v", v)
    end

    for k, v in ipairs(player[1].helloworld) do
        log_info("ipairs k", k, "v", v)
    end

    log_info("inspect player", pretty.dump(player))

    table.remove(player[1].helloworld)
    table.remove(player[1].helloworld)
    table.remove(player[1].helloworld)
    player[1].helloworld = nil
    player[1].helloworld = nil
    player[1] = nil
    -- testtb.helloworld = nil
    -- testtb.helloworld = 2
end)
