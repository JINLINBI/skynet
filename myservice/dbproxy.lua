local skynet = require "skynet.manager"
local settings = require "settings"
local inspect = require "inspect"

local skynet_node_name = ...

local CMD = {}


local function start()
    local conf = settings.nodes[skynet_node_name]
    if not conf then
        log_warning("没有找到[" .. skynet_node_name .. "] db配置，跳过初始化db...")
        return
    end

    log_warning("get node name information", inspect(conf))
    for _, proxy in ipairs(conf.dbproxy) do
        skynet.uniqueservice(proxy .. "pool", skynet_node_name)
    end
end

skynet.start(function()
    start()
    skynet.dispatch("lua", function(_, _, cmd, ...)
        local f = assert(CMD[cmd], cmd .. "not found")
        skynet.retpack(f(...))
    end)
    skynet.register('.' .. SERVICE_NAME)
end)