local msgserver = require "snax.msgserver"
local crypt = require "skynet.crypt"
local skynet = require "skynet"

local subid = 0
local server = {}
local servername 
function server.login_handler(uid, secret)
    skynet.error("login_handler invoke", uid, secret)
    subid = subid + 1
    local username = msgserver.username(uid, subid, servername)
    skynet.error("uid: ", uid, "login, username :", username)
    msgserver.login(username, secret)
    return subid
end



function server.logout_handler(uid, subid)
    skynet.error("logout_handler invoke", uid, subid)
    local username = msgserver.username(uid, subid, servername)
    msgserver.logout(username)
end


function server.kick_handler(uid, subid)
    skynet.error("kick_handler invoke", uid)
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
end

msgserver.start(server)
