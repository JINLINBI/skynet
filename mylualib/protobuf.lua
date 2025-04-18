local pb = require "pb"
local protoc = require "protoc"

local msgReqMap = {}  --msgId:msgName
local msgRespMap = {} --msgName:msgId

local M = {}


function M.load(fileName)
    local content = io.open(fileName, "r"):read("*a")
    protoc:load(content)
    M.parse()
end

function M.parse()
    for _, v in pb.types() do
        local sname, msgId = string.match(v, "(%a+)Req_(%d+)")
        if sname and msgId then
            msgReqMap[msgId] = sname
            msgReqMap[sname] = msgId
        end

        sname, msgId = string.match(v, "(%a+)Resp_(%d+)")
        if sname and msgId then
            msgRespMap[sname] = msgId
            msgRespMap[msgId] = sname
        end
    end
end

function M.encode(sname, data)
    local msgId = msgRespMap[sname]
    if msgId == nil then
        log_error("send a invalid msg", sname)
        return
    end

    local msgName = sname .. "Resp_" .. msgId
    local msg = pb.encode(msgName, data)
    if not msg then
        log_error("encode failed", msgId)
        return
    end

    return msgId, msg
end

function M.encodeClt(sname, data)
    local msgId = msgReqMap[sname]
    if msgId == nil then
        log_error("send a invalid msg ", sname)
        return
    end

    local msgName = sname .. "Req_" .. msgId
    local msg = pb.encode(msgName, data)
    if not msg then
        log_error("encodeClt failed", msgId)
        return
    end

    return msgId, msg
end

function M.decode(msgId, msg)
    msgId = tostring(msgId)
    local sname = msgReqMap[msgId]
    if not sname then
        log_error("decode recv a unkown msgid:", msgId, msgReqMap[msgId])
        return
    end
    local msgName = sname .. "Req_" .. msgId
    local data = pb.decode(msgName, msg)
    if not data then
        log_error("protobuf.decode failed a unkown msgid:", msgId, #msg)
        return
    end
    return sname, data
end

function M.decodeClt(msgId, msg)
    msgId = tostring(msgId)
    local sname = msgRespMap[msgId]
    if not sname then
        log_error("decodeClt recv a unkown msgid:", msgId)
        return
    end

    local msgName = sname .. "Resp_" .. msgId
    local data = pb.decode(msgName, msg)
    if not data then
        log_error("protobuf.decode failed a unkown msgid:", msgId, #msg)
        return
    end
    return sname, data
end

return M
