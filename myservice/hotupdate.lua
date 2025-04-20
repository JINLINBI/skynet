local skynet = require "skynet.manager" -- import skynet.register
local sharetable = require "skynet.sharetable"
local cache = require "skynet.codecache"
local version = require "version"
local uv = require "luv"
local lfs = require "lfs"
local protobuf = import("protobuf")
cache.mode "OFF"

local CHECK_FILE_SEC = 1 * 100   -- 1秒 检查文件事件的间隔
local DELAY_LOADTB_SEC = 3 * 100 -- 3秒 延迟加载tbconfig
local PBDIR = "proto/"
local TBDIR = "table/"
local IMPDIR = "./"

-- 监听项目里面所有的import的lua文件，除了table/
local function initImportWatch()
    local fs_event = uv.new_fs_event()
    -- 定义文件变化回调
    local function on_file_change(error, path, events)
        if error then
            log_error("[hotupdate import] uv watch err:", error)
            return
        end

        if not path:match(".lua") then return end

        -- 排除table配置文件夹
        if path:find(TBDIR) then return end

        for modname in path:gmatch("(%a+)%.lua") do
            local filename = IMPDIR .. path
            log_info("[hotupdate import]", filename, "was modified, update mod[", modname, "] version...")
            version.update(modname)
        end
    end

    -- 启动监听（递归监控子目录）
    uv.fs_event_start(fs_event, IMPDIR, { recursive = true }, on_file_change)
end

local function initProto()
    for entry in lfs.dir(PBDIR) do
        if not entry:match(".proto") then goto continue end

        local filename = PBDIR .. entry
        log_info("[hotupdate proto]", filename, "loading...")
        protobuf.load(filename)
        ::continue::
    end
end

local function CheckProtoSetup()
    local fs_event = uv.new_fs_event()
    -- 定义文件变化回调
    local function on_file_change(error, path, events)
        if error then
            log_error("[hotupdate proto] luv watch err:", error)
            return
        end

        if not path:match(".proto") then return end

        local filename = PBDIR .. path
        log_info("[hotupdate proto]", filename, "was modified, reloading...")
        local ok, err = pcall(protobuf.load, filename)
        if not ok then
            log_error("[hotupdate proto]", err)
        end
    end

    -- 启动监听（递归监控子目录）
    uv.fs_event_start(fs_event, PBDIR, {}, on_file_change)
end


local function initTbConfig()
    for entry in lfs.dir(TBDIR) do
        if not entry:match(".lua") then goto continue end

        local filename = TBDIR .. entry
        log_info("[hotupdate tbconfig]", filename, "loading...")
        sharetable.loadfile(filename)
        ::continue::
    end
end

local DelayUpdateTBs = {}
local function CheckTbconfigSetup()
    local fs_event = uv.new_fs_event()

    -- 定义文件变化回调
    local function on_file_change(error, path, events)
        if error then
            log_error("[hotupdate tbconfig] luv watch err:", error)
            return
        end

        if not path:match(".lua") then return end

        local filename = TBDIR .. path
        log_info("[hotupdate tbconfig] detecting", filename, "was modified, update version after 3 seconds...")
        table.insert(DelayUpdateTBs, filename)
    end

    -- 启动监听（递归监控子目录）
    uv.fs_event_start(fs_event, TBDIR, {}, on_file_change)
end

local function StartDelayUpdateTbconfigs()
    skynet.fork(function(tbs)
        skynet.sleep(DELAY_LOADTB_SEC)
        for _, filename in pairs(tbs) do
            log_info("[hotupdate tbconfig]", filename, "was modified, update file version...")
            version.update(filename)
        end
    end, DelayUpdateTBs)
end

skynet.start(function()
    skynet.register("hotupdate") -- 注册服务名

    initImportWatch()
    initProto()
    initTbConfig()
    CheckProtoSetup()
    CheckTbconfigSetup()

    skynet.fork(function()
        while true do
            skynet.sleep(CHECK_FILE_SEC)
            uv.run("nowait")

            if #DelayUpdateTBs ~= 0 then
                StartDelayUpdateTbconfigs()
                DelayUpdateTBs = {}
            end
        end
    end)
end)
