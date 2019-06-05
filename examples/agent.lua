local skynet = require "skynet"
local socket = require "skynet.socket"
local mysql = require "skynet.db.mysql"
local sproto = require "sproto"
local sprotoloader = require "sprotoloader"
local crypt = require "skynet.crypt"

local WATCHDOG
local host
local send_request

local CMD = {}
local REQUEST = {}
local client_fd

local function dump(obj)
    local getIndent, quoteStr, wrapKey, wrapVal, dumpObj
    getIndent = function(level)
        return string.rep("\t", level)
    end
    quoteStr = function(str)
        return '"' .. string.gsub(str, '"', '\\"') .. '"'
    end
    wrapKey = function(val)
        if type(val) == "number" then
            return "[" .. val .. "]"
        elseif type(val) == "string" then
            return "[" .. quoteStr(val) .. "]"
        else
            return "[" .. tostring(val) .. "]"
        end
    end
    wrapVal = function(val, level)
        if type(val) == "table" then
            return dumpObj(val, level)
        elseif type(val) == "number" then
            return val
        elseif type(val) == "string" then
            return quoteStr(val)
        else
            return tostring(val)
        end
    end
    dumpObj = function(obj, level)
        if type(obj) ~= "table" then
            return wrapVal(obj)
        end
        level = level + 1
        local tokens = {}
        tokens[#tokens + 1] = "{"
        for k, v in pairs(obj) do
            tokens[#tokens + 1] = getIndent(level) .. wrapKey(k) .. " = " .. wrapVal(v, level) .. ","
        end
        tokens[#tokens + 1] = getIndent(level - 1) .. "}"
        return table.concat(tokens, "\n")
    end
    return dumpObj(obj, 0)
end

function REQUEST:get()
	print("get", self.what)
	local r = skynet.call("SIMPLEDB", "lua", "get", self.what)
	return { result = r }
end

function REQUEST:set()
	print("set", self.what, self.value)
	local r = skynet.call("SIMPLEDB", "lua", "set", self.what, self.value)
end

function REQUEST:handshake()
	return { msg = "Welcome to skynet, I will send heartbeat every 5 sec." }
end

function REQUEST:quit()
	skynet.call(WATCHDOG, "lua", "close", client_fd)
end

function REQUEST:login()
	local name = string.match(self.name, '%w+')
	local password = string.match(self.password, '[%w,.]+')
	print(name, " requests login.")
	local ret = db:query(string.format("select * from person where name = '%s'", name))
	print(dump(ret))
	if ret.err then
		print(ret.err)
		return {result = -1, msg = "login error!"}
	else
		if crypt.hexencode(crypt.sha1(password)) == ret[1].password then
			return {result = -1, msg = "login suc!"}
		else
			return {result = -1, msg = "login failed!"}
		end
	end
	
end

function REQUEST:register()
	local name = string.match(self.name, '%w+')
	local password = string.match(self.password, '[%w,.]+')
	local ret = db:query("select * from person where name = '%s'", name)

	print("registering ", name, password)
	print(dump(ret))
	if ret == nil then
		password = crypt.hexencode(crypt.sha1(password))
		local ret = db:query(string.format("insert into person(name, password) values('%s','%s')", name, password))
		print(dump(ret))
		--[[
		elseif name == nil or ret[1].name then
			return { result = -1, msg = "user exists."}
		elseif ret.err then
			return { result = -1, msg = "database error."}
		--]]
	else
		password = crypt.hexencode(crypt.sha1(password))
		local ret = db:query(string.format("insert into person(name, password) values('%s','%s')", name, password))
		print(dump(ret))
	end
	
end

local function request(name, args, response)
	local f = assert(REQUEST[name])
	local r = f(args)
	if response then
		return response(r)
	end
end

local function send_package(pack)
	local package = string.pack(">s2", pack)
	socket.write(client_fd, package)
end

skynet.register_protocol {
	name = "client",
	id = skynet.PTYPE_CLIENT,
	unpack = function (msg, sz)
		return host:dispatch(msg, sz)
	end,
	dispatch = function (fd, _, type, ...)
		assert(fd == client_fd)	-- You can use fd to reply message
		skynet.ignoreret()	-- session is fd, don't call skynet.ret
		if type == "REQUEST" then
			local ok, result  = pcall(request, ...)
			if ok then
				if result then
					send_package(result)
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
	host = sprotoloader.load(1):host "package"
	send_request = host:attach(sprotoloader.load(2))

	client_fd = fd
	skynet.call(gate, "lua", "forward", fd)
end

function CMD.disconnect()
	-- todo: do something before exit
	skynet.exit()
end

skynet.start(function()
	skynet.dispatch("lua", function(_,_, command, ...)
		local f = CMD[command]
		skynet.ret(skynet.pack(f(...)))
	end)

	local function on_connect(db)
		db:query("set charset utf8");
	end

	db = mysql.connect({
		host = "127.0.0.1",
		port = 3306,
		database = "test",
		user = "testuser",
		password = "password",
		max_packet_size = 1024 * 1024,
		on_connect = on_connect
	})
end)
