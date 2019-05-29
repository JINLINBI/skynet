local msgserver = require "snax.msgserver"
local crypt = require "skynet.crypt"
local skynet = require "skynet"

local loginservice = tonumber(...)
local server = {}
local servername 
local subid = 0


function server.login_handler(uid, secret)
    subid = subid + 1
    local username = msgserver.username(uid, subid, servername)
    skynet.error("uid: ", uid, "login, username :", username)
    msgserver.login(username, secret)
    return subid
end


function server.logout_handler(uid, subid)
    local username = msgserver.username(uid, subid, servername)
    msgserver.logout(username)
end


function server.kick_handler(uid, subid)
    server.logout_handler(uid, subid)
end


function server.disconnect_handler(username)
    skynet.error(username, "disconnect")
end


function server.request_handler(username, msg)
    skynet.error("recv", msg, "from", username)
    return string.upper(msg)
end


function server.register_handler(name)
    skynet.error("register_handler invoked name: ", name)
    servername = name
    skynet.call(loginservice, "lua", "register_gate", servername, skynet.self())
end

msgserver.start(server)
