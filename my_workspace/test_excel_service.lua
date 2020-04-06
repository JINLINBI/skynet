local skynet = require("skynet")
require "skynet.manager"

local REG = skynet.register_protocol
REG {
	name = "text",
	id = skynet.PTYPE_TEXT,
	pack = skynet.pack,
	unpack = skynet.unpack,
}

skynet.start(function()
	local service = skynet.localname(".excel")

	skynet.send(service, "text", "hello!!!!")
end)
