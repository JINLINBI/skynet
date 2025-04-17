package.cpath = "luaclib/?.so;myluaclib/?.lua"
package.path = "lualib/?.lua;examples/?.lua;mylualib/?.lua"

if _VERSION ~= "Lua 5.4" then
	error "Use lua 5.4"
end

local socket = require "client.socket"
local protobuf = require "pb"
local inspect = require "inspect"
local pb = require "mypb"
local cjson = require "cjson.util"

local targetFile = "proto/msg.proto"

pb.LoadProtoFile(targetFile)
pb.ParseProtoFile(targetFile)

-- for k, v in protobuf.types() do
-- 	print("k", k, "v", v, "type", type(v))
-- 	for k, v in protobuf.fields(v) do
-- 		print(k, v)
-- 	end
-- end
function string_to_hex(str)
    local hex = {}
    for i = 1, #str do
        local byte = string.byte(str, i)
        hex[i] = string.format("%02X", byte)  -- %02X 确保两位大写十六进制
    end
    return print(table.concat(hex, " "))  -- 可选空格分隔
end

-- 示例：输出 "Hello" 的十六进制表示（48 65 6C 6C 6F）
-- print(string_to_hex("Hello")) --> "48 65 6C 6C 6F"

-- 示例：打印 "AB" 的16位二进制
-- print(print_binary_hex("AB"))  --> "01000001 01000010"
-- local proto = require "proto"
-- local sproto = require "sproto"

-- local host = sproto.new(proto.s2c):host "package"
-- local request = host:attach(sproto.new(proto.c2s))

local fd = assert(socket.connect("127.0.0.1", 8888))

local function send_package(fd, msgName, msgData)
	print("fd", fd, "msgName", msgName, "msgData", msgData)
	local msgId, encodeData = pb.PbEncodeClt(msgName, msgData or {})
	print("pbdata len", #encodeData)
	local package = string.pack(">HI", #encodeData + 4, msgId) .. (encodeData or "")
	print("finallen", #package)
	string_to_hex(package)
	socket.send(fd, package)
end

local function unpack_package(text)
	local size = #text
	if size < 2 then
		return nil, text
	end
	local s = text:byte(1) * 256 + text:byte(2)
	if size < s + 2 then
		return nil, text
	end

	return text:sub(3, 2 + s), text:sub(3 + s)
end

local function recv_package(last)
	local result
	result, last = unpack_package(last)
	if result then
		return result, last
	end
	local r = socket.recv(fd)
	if not r then
		return nil, last
	end
	if r == "" then
		error "Server closed"
	end
	return recv_package(last .. r)
end

local session = 0

local function send_request(name, args)
	session = session + 1
	-- local str = request(name, args, session)
	send_package(fd, name, args)
	print("Request:", session)
end

local last = ""

local function print_request(name, args)
	print("REQUEST", name)
	if args then
		for k, v in pairs(args) do
			print(k, v)
		end
	end
end

local function print_response(session, args)
	print("RESPONSE", session)
	if args then
		for k, v in pairs(args) do
			print(k, v)
		end
	end
end

local function print_package(t, ...)
	if t == "REQUEST" then
		print_request(...)
	else
		assert(t == "RESPONSE")
		print_response(...)
	end
end

local function UnpackData(msgStr)
	print("msglen", #msgStr)
	local msgId, pos = string.unpack(">I", msgStr)
	local msgName, pbData = pb.PbDecodeClt(msgId, msgStr:sub(pos))
	print("msgName", msgName, "pbData", pbData, "pbData.msg", pbData.msg)
	print("msgId", msgId, "pbStr", pbData, "pos", pos, msgName)
	return msgName, pbData
end

local function dispatch_package()
	while true do
		local v
		v, last = recv_package(last)
		if not v then
			break
		end

		print_package("RESPONSE", UnpackData(v))
	end
end


send_request("Handshake", { msg = "test" })
-- send_request("Set", { msg = "test" })
send_request("Set", { what = "hello", value = "world" })
while true do
	dispatch_package()
	local cmd = socket.readstdin()
	if cmd then
		if cmd == "quit" then
			send_request("quit")
		else
			send_request("test", { what = cmd })
		end
	else
		socket.usleep(100)
	end
end
