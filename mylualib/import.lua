-- 项目里面的import函数模仿实现
-- 定义一个函数，加载并执行文件，并捕获全局函数到 _G

local skynet = require "skynet"

local M = {}

local ImportCache = {}
local IMPORT_CHECK_SEC = 3

local function IndexFunction(t, k)
    local realmod = rawget(t, "__realmod")
    if not realmod then return end

    return realmod[k]
end

local function NewIndexFunction(t, k, v)
    skynet.error("must not change import mod", t.__modname)
end

function M.import(filename)
    local mod = ImportCache[filename]
    if mod then
        return mod
    end

    -- print("import!!!!!!!!!!!!!!!!!")
    local realmod = require(filename)
    if not realmod then
        skynet.error("import not founded !!!!!!!!!!!!!!!!!")
        return realmod
    end

    local newmod = {
        __modname = filename,
        __version = 0,
        __sd = nil,
        __realmod = realmod,
    }

    setmetatable(newmod, {
        __index = IndexFunction,
        __newindex = NewIndexFunction,
    })

    ImportCache[filename] = newmod
    return newmod
end

skynet.fork(function ()
    skynet.sleep(IMPORT_CHECK_SEC * 100)

    while true do
        local sharedata = require "skynet.sharedata"
        for _, mod in pairs(ImportCache) do
            local sd = rawget(mod, "__sd")
            local ver = rawget(mod, "__version")
            if not sd or (sd.version ~= ver) then
                if not sd then
                    sd = sharedata.query("hotupdate_ver")
                    rawset(mod, "__sd", sd)
                    rawset(mod, "__version", sd.version)
                end
                skynet.error("update mod", mod.__modname)
                package.loaded[mod.__modname] = nil
                local newmod = require(mod.__modname)
                rawset(mod, "__realmod", newmod)
                rawset(mod, "__version", sd.version)
            end
        end
        skynet.sleep(IMPORT_CHECK_SEC * 100)
    end
end)

return M