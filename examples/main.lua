local skynet = require "skynet"
local settings = require "settings"

local max_client = 64

skynet.start(function()
	-- skynet.error("Server start")
	skynet.error("Gameserver start server")
    local node_name  = skynet.getenv("node_name")

    INFO("-----GameServer-----", node_name, " will begin")
    local cfg = settings.nodes[tostring(node_name)]

	skynet.uniqueservice("hotupdate")
	skynet.uniqueservice("dbproxy", cfg.node_name)
	if not skynet.getenv "daemon" then
		local console = skynet.newservice("console")
	end
	skynet.newservice("debug_console", 11111)
	skynet.newservice("simpledb")
	-- skynet.newservice("testprotobuf")
	-- skynet.newservice("testplayerId")
	-- -- skynet.newservice("testtb")
	-- skynet.newservice("testversion")
	-- skynet.newservice("testconfloader")
	-- skynet.newservice("testkvstore")
	-- skynet.newservice("testimport")
	-- skynet.newservice("testetcd")
	-- skynet.newservice("testluasql")
	-- skynet.newservice("testredisdb")
	-- skynet.newservice("testplayerdata")
	skynet.newservice("testdatamanager")

	local watchdog = skynet.newservice("watchdog")
	local addr, port = skynet.call(watchdog, "lua", "start", {
		port = 8888,
		maxclient = max_client,
		nodelay = true,
	})
	skynet.error("Watchdog listen on " .. addr .. ":" .. port)
	skynet.exit()
end)
