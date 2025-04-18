local skynet = require "skynet.manager" -- import skynet.register
local sharedata = require "skynet.sharedata"
local sharetable = require "skynet.sharetable"
local inotify = require("inotify")
local lfs = require("lfs")
local protobuf = import("protobuf")
local CHECK_FILE_SEC = 3 * 100 -- 检查文件事件的间隔
local PBDIR = 'proto/'
local TBDIR = 'table/'

local function initProto()
    for entry in lfs.dir(PBDIR) do
        -- print("lfs list dir: ", entry)
        if not entry:match(".proto") then
            goto continue
        end

        local filename = PBDIR .. entry
        log_info("[protobuf] " .. filename .. ' loading...')
        protobuf.load(filename)
        ::continue::
    end
end

local function CheckProtoThread()
    local handle = inotify.init({ blocking = false })
    handle:addwatch(PBDIR, inotify.IN_MODIFY, inotify.IN_CREATE, inotify.IN_MOVED_TO)
    while true do
        for _, ev in pairs(handle:read()) do
            local filename = PBDIR .. ev.name
            log_info("[protobuf] " .. filename .. ' was modify, reloading...')
            local ok, err = pcall(protobuf.load, filename)
            if not ok then
                log_info(err)
            end
        end
        skynet.sleep(CHECK_FILE_SEC)
    end
end


local function initTbConfig()
    for entry in lfs.dir(TBDIR) do
        -- print("lfs list dir: ", entry)
        if not entry:match(".lua") then
            goto continue
        end

        local filename = TBDIR .. entry
        log_info("[tbconfig] " .. filename .. ' loading...')
        sharetable.loadfile(filename)
        ::continue::
    end
end

local function CheckTbconfigThread()
    local handle = inotify.init({ blocking = false })
    handle:addwatch(TBDIR, inotify.IN_MODIFY, inotify.IN_CREATE, inotify.IN_MOVED_TO)
    while true do
        for _, ev in pairs(handle:read()) do
            local filename = TBDIR .. ev.name
            log_info("[tbconfig] " .. filename .. ' was modify, reloading...')
            sharetable.loadfile(filename)
        end
        skynet.sleep(CHECK_FILE_SEC)
    end
end

skynet.start(function()
    skynet.register("hotupdate") -- 注册服务名
    sharedata.update("hotupdate_ver", { version = skynet.time() })
    skynet.dispatch("lua", function(_, _, cmd)
        log_info("noticed clearcache, updating import lua...", cmd)
        sharedata.update("hotupdate_ver", { version = skynet.time() })
    end)

    initProto()
    initTbConfig()
    skynet.fork(CheckProtoThread)
    skynet.fork(CheckTbconfigThread)
end)
