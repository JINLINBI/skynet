local pb = require "pb"
local protoc = require "protoc"

local msgReqMap = {}  --msgId:msgName
local msgRespMap = {} --msgName:msgId

local M = {}


function M.LoadProtoFile(fileName)
    local proto_content = io.open(fileName, "r"):read("*a")
    protoc:load(proto_content)
    M.ParseProto()
end


function M.ParseProto()
    for k, v in pb.types() do
        local msgName, id = string.match(v, "(%a+)Req_(%d+)")
        if msgName and id then
            -- print("k", k, "v", v, "type", type(v))
            -- print(v)
            msgReqMap[id] = msgName
            msgReqMap[msgName] = id
            -- msgRespCltMap[msgName] = msgId
        end

        msgName, id = string.match(v, "(%a+)Resp_(%d+)")
        if msgName and id then
            -- print("k", k, "v", v, "type", type(v))
            -- print(v)
            msgRespMap[msgName] = id
            msgRespMap[id] = msgName
            -- msgReqCltMap[msgId] = msgName
        end
    end
end


function M.PbEncode(msgName, msgBody)
    local msgid = msgRespMap[msgName]
    if msgid == nil then
        print("send a invalid msg", msgName)
        return
    end
    local msgPbName = msgName .. "Resp_" .. msgid
    -- log_info("start encode ",msgPbName, msgid)
    local msgData = pb.encode(msgPbName, msgBody)
    if not msgData then
        print("decode failed", msgid)
        return
    end

    return msgid, msgData
end

function M.PbEncodeClt(msgName, msgBody)
    local id = msgReqMap[msgName]
    if id == nil then
        print("send a invalid msg ", msgName)
        return
    end
    local msgPbName = msgName .. "Req_" .. id
    -- log_info("start encode ",msgPbName, msgid)

    local msgData = pb.encode(msgPbName, msgBody)
    if not msgData then
        print("decode failed", id)
        return
    end

    return id, msgData
end


function M.PbDecode(id, msgData)
    id = tostring(id)
    -- log_info("start decode",msgId, #msgData)
    local namePre = msgReqMap[id]
    if not namePre then
        print("PbDecode recv a unkown msgid:", id, msgReqMap[id])
        return
    end
    local msgName = namePre .. "Req_" .. id
    local msgBody = pb.decode(msgName, msgData)
    if not msgBody then
        print("pb.decode failed a unkown msgid:", id, #msgData)
        return
    end
    return namePre, msgBody
end

function M.PbDecodeClt(id, msgData)
    id = tostring(id)
    -- log_info("start decode",msgId, #msgData)
    local namePre = msgRespMap[id]
    if not namePre then
        print("PbDecode recv a unkown msgid:", id)
        return
    end
    local msgName = namePre .. "Resp_" .. id
    local msgBody = pb.decode(msgName, msgData)
    if not msgBody then
        print("pb.decode failed a unkown msgid:", id, #msgData)
        return
    end
    return namePre, msgBody
end

return M