local skynet = require "skynet"

local max_client = 64

skynet.start(function()
	skynet.error("Server start")
	skynet.uniqueservice("hotupdate")
	if not skynet.getenv "daemon" then
		local console = skynet.newservice("console")
	end
	skynet.newservice("debug_console", 11111)
	skynet.newservice("simpledb")
	skynet.newservice("testprotobuf")
	skynet.newservice("testplayerId")
	-- skynet.newservice("testtb")
	skynet.newservice("testversion")
	skynet.newservice("testconfloader")
	skynet.newservice("testkvstore")
	skynet.newservice("testimport")

	local watchdog = skynet.newservice("watchdog")
	local addr, port = skynet.call(watchdog, "lua", "start", {
		port = 8888,
		maxclient = max_client,
		nodelay = true,
	})
	skynet.error("Watchdog listen on " .. addr .. ":" .. port)
	skynet.exit()
end)
