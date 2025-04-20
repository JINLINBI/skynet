-- 项目里面的import函数模仿实现
-- 定义一个函数，加载并执行文件，并捕获全局函数到 _G

local skynet = require "skynet"
local version = require "version"
local table_ex = require "table_ex"
local enableHotUpdate = skynet.getenv("autohotupdated") or false

local M = {}

local ImportCache = {}
local IMPORT_CHECK_SEC = 1 * 100 -- 0.5秒检测一次
local DEALY_IMPORT_SEC = 3 * 100 -- 3秒延迟加载

local function IndexFunction(t, k)
    local mod = rawget(t, "mod")
    if not mod then return end

    return mod[k]
end

local function NewIndexFunction(t, k, v)
    skynet.error("must not change import mod", t.filename)
end

if not enableHotUpdate then
    M.import = require
else
    function M.import(filename)
        local mod = ImportCache[filename]
        if mod then return mod end

        local realmod = require(filename)
        if not realmod then
            log_error("import not founded !!!!!!!!!!!!!!!!!")
            return realmod
        end

        local newmod = {
            filename = filename,
            ver = version.get(filename),
            mod = realmod,
        }

        setmetatable(newmod, {
            __index = IndexFunction,
            __newindex = NewIndexFunction,
        })

        ImportCache[filename] = newmod
        log_info("import get new mod, ", ImportCache, "filename", filename, "newmod", newmod)
        return newmod
    end

    if not _G.IMPORT_CHECKCO then
        _G.IMPORT_CHECKCO = skynet.fork(function()
            local lastHotUpdateVer = version.get("hotupdate_ver")
            while true do
                skynet.sleep(IMPORT_CHECK_SEC)
                local newversion = version.get("hotupdate_ver")
                if lastHotUpdateVer ~= newversion then
                    lastHotUpdateVer = newversion

                    -- log_info("[import] detecting clearcache, delay import after 3 secs...")
                    skynet.sleep(DEALY_IMPORT_SEC)

                    local count = 0
                    for _, mod in pairs(ImportCache) do
                        local ver      = rawget(mod, "ver")
                        local filename = rawget(mod, "filename")
                        local newver   = version.get(filename)
                        if newver ~= ver then
                            package.loaded[filename] = nil
                            local ok, newmod = pcall(require, filename)
                            if ok then
                                count = count + 1
                                rawset(mod, "mod", newmod)
                                rawset(mod, "ver", newver)
                                log_info("[import] update mod", filename, "successed")
                            else
                                log_error("[import]", newmod)
                            end
                        end
                    end

                    -- log_info("[import] reimport ", count, "modules")
                end
            end
        end)
    end
end

return M
