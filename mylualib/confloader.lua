local sharetable = require "skynet.sharetable"
local version = require "version"

local M = {
    cacheList = {},
}

M.__index = M

local function checkUpdate(o, k)
    local filename = rawget(o, "filename")
    local ver = rawget(o, "version")
    local newver = version.get(filename)
    if ver ~= newver then
        log_info("version ~= ver", version.get(filename), ver)
        sharetable.loadfile(filename)
        sharetable.update(filename)
        rawset(o, "version", newver)
        for _, cb in pairs(rawget(o, "updatecblist")) do
            cb(rawget(o, "t"))
        end
    end

    return rawget(o, "M")[k]
end


function M.new(filename, updatecb)
    local o = M.cacheList[filename]
    if o then
        table.insert(o.updatecblist, updatecb)
        return M.cacheList[filename]
    end

    local t = sharetable.query(filename)
    if not t then return end

    o = setmetatable({
        t = t,
        version = version.get(filename),
        filename = filename,
        updatecblist = {},
        M = M,
    }, { __index = checkUpdate, __newindex = function() end })

    table.insert(o.updatecblist, updatecb)
    M.cacheList[filename] = o
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
    if not sheet then return end

    for _, conf in pairs(sheet) do
        local bk, ret = callback(conf)
        if bk then return ret end
    end
end

return M
