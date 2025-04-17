local skynet = require "skynet"
local inspect = require "inspect"
local socket = require "skynet.socket"

local pb = require "mypb"

local testhotupdate = import("testhotupdate")

local WATCHDOG
local host
local send_request

local CMD = {}
local REQUEST = {}
local client_fd

function REQUEST:get()
	print("get", self.what)
	local r = skynet.call("SIMPLEDB", "lua", "get", self.what)
	return "Get", { result = r }
end

function REQUEST:set()
	print("set", self.what, self.value)
	-- local r = skynet.call("SIMPLEDB", "lua", "set", self.what, self.value)
	print("ok?", self.what, self.value)
end

function REQUEST:handshake()
	return "Handshake", { msg = "Welcome to skynet, I will send heartbeat every 5 sec." }
end

function REQUEST:quit()
	skynet.call(WATCHDOG, "lua", "close", client_fd)
end

function REQUEST:test()
	testhotupdate.test()
end

local function request(name, args, response)
	local f = assert(REQUEST[name])
	local r = f(args)
	-- if response then
	-- 	return response(r)
	-- end
end

local function send_package(msgName, msgData)
	print("send_package", msgName)
	local msgId, encodeData = pb.PbEncode(msgName, msgData or {})
	print("pbEncode", msgId)
	local package = string.pack(">HI", #encodeData + 4, msgId) .. (encodeData or "")
	print("package", #package)
	string_to_hex(package)
	socket.write(client_fd, package)
end

function string_to_hex(str)
    local hex = {}
    for i = 1, #str do
        local byte = string.byte(str, i)
        hex[i] = string.format("%02X", byte)  -- %02X 确保两位大写十六进制
    end
    return print(table.concat(hex, " "))  -- 可选空格分隔
end

local function safe_sub(str, from, to)
    if from > #str or to > #str then
        skynet.error("Invalid substring range:", from, to, #str)
        return ""
    end
    return str:sub(from, to)
end

skynet.register_protocol {
	name = "client",
	id = skynet.PTYPE_CLIENT,
	unpack = function (msg, sz)
		local msgStr = skynet.tostring(msg, sz)
		string_to_hex(msgStr)
		local msgId, pos = string.unpack(">I", msgStr)
		local pbStr = safe_sub(msgStr, 5, #msgStr)
		print("msgId", msgId, "pbStr", pbStr, "pos", pos)
		local msgName, msgBody = pb.PbDecode(msgId, pbStr)
		print(msgName, inspect(msgBody))

		return "REQUEST", string.lower(msgName), msgBody
	end,

	dispatch = function (fd, _, type, ...)
		assert(fd == client_fd)	-- You can use fd to reply message
		skynet.ignoreret()	-- session is fd, don't call skynet.ret
		skynet.trace()
		if type == "REQUEST" then
			local ok, name, result  = pcall(request, ...)
			if ok then
				if result then
					send_package(name, result)
				end
			else
				skynet.error(result)
			end
		else
			assert(type == "RESPONSE")
			error "This example doesn't support request client"
		end
	end
}

function CMD.start(conf)
	local fd = conf.client
	local gate = conf.gate
	WATCHDOG = conf.watchdog
	-- slot 1,2 set at main.lua
	-- host = sprotoloader.load(1):host "package"
	-- send_request = host:attach(sprotoloader.load(2))


	skynet.fork(function()
		while true do
			send_package("HeartBeat", {msg = "helloworld"})
			skynet.sleep(500)
		end
	end)

	client_fd = fd
	skynet.call(gate, "lua", "forward", fd)
end

function CMD.disconnect()
	-- todo: do something before exit
	skynet.exit()
end

skynet.start(function()

	local targetFile = "proto/msg.proto"

    pb.LoadProtoFile(targetFile)
    pb.ParseProtoFile(targetFile)

	skynet.dispatch("lua", function(_,_, command, ...)
		skynet.trace()
		local f = CMD[command]
		skynet.ret(skynet.pack(f(...)))
	end)
end)
