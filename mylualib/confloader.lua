local skynet = require "skynet"
local sharetable = require "skynet.sharetable"

-- sharetable.loadfile("table/example.lua")
-- local t = sharetable.query("table/example.lua")

local CHECK_UPDATE_SEC = 5
local M = {}
M.__index = M

local function checkUpdate(o, k)
    local now = skynet.time()
    if now - rawget(o, "last") > CHECK_UPDATE_SEC then
        -- log_info("checkUpdate", k)
        local filename = rawget(o, "filename")
        log_info("update?", sharetable.update(filename))
        rawset(o, "last", now)
    end

    return rawget(o, "M")[k]
end


function M.new(filename, callback)
    local t = sharetable.query(filename)
    if not t then return end

    local o = setmetatable({
        t = t,
        last = skynet.time(),
        filename = filename,
        callback = callback,
        M = M,
    }, {__index = checkUpdate, __newindex = function () end})
    return o
end

function M:get(sheetName, index)
    local sheet = self.t[sheetName]
    if not sheet then return end

    return sheet[index]
end

function M:count(sheetName)
    return self.t[sheetName .. "_num"] or 0
end

function M:getByIndex(sheetName, indexField, idx)
    local sheet = self.t[sheetName]
    if not sheet then return end

    local dualIdx = self.t[sheetName .. "_" .. indexField]
    if not dualIdx then return end

    local id = dualIdx[idx]
    if not id then return end
    return sheet[id]
end

function M:foreach(sheetName, callback)
    local sheet = self.t[sheetName]
    for _, conf in pairs(sheet) do
        local bk, ret = callback(conf)
        if bk then
            return ret
        end
    end
end

return M
