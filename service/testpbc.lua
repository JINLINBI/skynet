local skynet = require "skynet"
local protobuf = require "protobuf"


skynet.start(function()
	protobuf.register_file"./protos/test.pb"
	skynet.error("protobuf register: test.pb")
	
	stringbuffer = protobuf.encode("cs.test",
	{
		name = "xiaming",
		age = 1,
		online = true,
		account = 888.88,
	})

	local data = protobuf.decode("cs.test", stringbuffer)
	skynet.error("-----decode------\nname=", data.name
	,",\nage=", data.age
	,",\nemail=", data.email
	,",\nonline=", data.online
	,",\naccount=", data.account)
end)
