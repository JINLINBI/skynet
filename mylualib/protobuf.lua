local pb = require "pb"
local protoc = require "protoc"
local kv = require "kvstore"

local M = {}

function M.SetMsgReqMsp(k, v)
    return kv.set("PReq" .. k, v)
end

function M.GetMsgReqMsp(k)
    return kv.get("PReq" .. k)
end

function M.SetMsgRespMsp(k, v)
    return kv.set("PRsp" .. k, v)
end

function M.GetMsgRespMsp(k)
    return kv.get("PRsp" .. k)
end

function M.load(fileName)
    local content = io.open(fileName, "r"):read("*a")
    protoc:load(content)
    M.parse()
end

function M.parse()
    for _, v in pb.types() do
        local sname, msgId = string.match(v, "(%a+)Req_(%d+)")
        if sname and msgId then
            M.SetMsgReqMsp(msgId, sname)
            M.SetMsgReqMsp(sname, msgId)
        end

        sname, msgId = string.match(v, "(%a+)Resp_(%d+)")
        if sname and msgId then
            M.SetMsgRespMsp(sname, msgId)
            M.SetMsgRespMsp(msgId, sname)
        end
    end
end

function M.encode(sname, data)
    local msgId = M.GetMsgRespMsp(sname)
    if msgId == nil then
        log_error("[protobuf] send a invalid msg", sname)
        return
    end

    local msgName = sname .. "Resp_" .. msgId
    local msg = pb.encode(msgName, data)
    if not msg then
        log_error("[protobuf] encode failed", msgId)
        return
    end

    return msgId, msg
end

function M.encodeClt(sname, data)
    local msgId = M.GetMsgReqMsp(sname)
    if msgId == nil then
        log_error("[protobuf] send a invalid msg ", sname)
        return
    end

    local msgName = sname .. "Req_" .. msgId
    local msg = pb.encode(msgName, data)
    if not msg then
        log_error("[protobuf] encodeClt failed", msgId)
        return
    end

    return msgId, msg
end

function M.decode(msgId, msg)
    msgId = tostring(msgId)
    local sname = M.GetMsgReqMsp(msgId)
    if not sname then
        log_error("[protobuf] decode recv a unkown msgid:", msgId)
        return
    end
    local msgName = sname .. "Req_" .. msgId
    local data = pb.decode(msgName, msg)
    if not data then
        log_error("[protobuf] protobuf.decode failed a unkown msgid:", msgId, #msg)
        return
    end
    return sname, data
end

function M.decodeClt(msgId, msg)
    msgId = tostring(msgId)
    local sname = M.GetMsgRespMsp(msgId)
    if not sname then
        log_error("[protobuf] decodeClt recv a unkown msgid:", msgId)
        return
    end

    local msgName = sname .. "Resp_" .. msgId
    local data = pb.decode(msgName, msg)
    if not data then
        log_error("[protobuf] protobuf.decode failed a unkown msgid:", msgId, #msg)
        return
    end
    return sname, data
end

return M
