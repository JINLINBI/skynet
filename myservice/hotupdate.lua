local skynet = require "skynet.manager" -- import skynet.register
local sharedata = require "skynet.sharedata"
local inotify = require("inotify")
local lfs = require("lfs")
local protobuf = import("protobuf")
local CHECK_FILE_SEC = 3 * 100 -- 检查文件事件的间隔


skynet.start(function()
    skynet.register("hotupdate") -- 注册服务名
    sharedata.update("hotupdate_ver", { version = skynet.time() })
    skynet.dispatch("lua", function(_, _, cmd)
        print("hot update version !!!!!!!!!!!!!", cmd)
        sharedata.update("hotupdate_ver", { version = skynet.time() })
    end)


    skynet.fork(function()
        local handle = inotify.init({ blocking = false })
        local PBDIR = 'proto/'
        for entry in lfs.dir(PBDIR) do
            -- print("lfs list dir: ", entry)
            if not entry:match(".proto") then
                goto continue
            end

            local filename = PBDIR .. entry
            skynet.error("[protobuf] " .. filename .. ' loading...')
            protobuf.load(filename)
            ::continue::
        end

        handle:addwatch(PBDIR, inotify.IN_MODIFY, inotify.IN_CREATE, inotify.IN_MOVED_TO)
        while true do
            for _, ev in pairs(handle:read()) do
                local filename = PBDIR .. ev.name
                skynet.error("[protobuf] " .. filename .. ' was modify, reloading...')
                local ok, err = pcall(protobuf.load, filename)
                if not ok then
                    skynet.error(err)
                end
            end
            skynet.sleep(CHECK_FILE_SEC)
        end
    end)
end)
