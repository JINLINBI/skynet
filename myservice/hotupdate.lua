local skynet = require "skynet"
require "skynet.manager"	-- import skynet.register
local sharedata = require "skynet.sharedata"
local inotify = require("inotify")
print("inotify ?????", inotify)


skynet.start(function()
    skynet.register("hotupdate")  -- 注册服务名
    sharedata.update("hotupdate_ver", { version = skynet.time() })
    skynet.dispatch("lua", function(_, _, cmd)

        print("hot update version !!!!!!!!!!!!!", cmd)
        sharedata.update("hotupdate_ver", { version = skynet.time() })
    end)

    -- 监视单个文件的所有事件
    local watcher = inotify.new()
    local watchPath = "proto"
    watcher:watch(watchPath)

    -- 监视一个目录的特定事件
    watcher:watch(watchPath, {
        create = true,    -- 创建文件/目录
        delete = true,    -- 删除文件/目录
        modify = true,    -- 文件内容被修改
        move_self = true, -- 被监视的文件/目录自身被移动
        moved_from = true, -- 文件从被监视的目录移出
        moved_to = true,   -- 文件被移动到被监视的目录
        open = true,      -- 文件被打开
        close_write = true, -- 以写模式打开的文件被关闭
        close_nowrite = true, -- 未以写模式打开的文件被关闭
        access = true,    -- 文件被访问 (读取)
        attrib = true,    -- 文件元数据被改变 (例如，权限，时间戳)
        unmount = true,   -- 包含被监视对象的 文件系统被卸载
        close = true      -- close_write 和 close_nowrite 的简写
    })

    -- -- 你也可以使用 inotify 模块中定义的常量来表示事件
    -- local events = inotify.IN_CREATE + inotify.IN_DELETE + inotify.IN_MODIFY
    -- watcher:watch("/another/path", events)
    skynet.fork(function ()
        -- 阻塞读取: 等待至少一个事件发生
        local events = watcher:read()

        -- 非阻塞读取: 立即返回，可能返回一个空表
        -- local events = watcher:read(0)

        if events then
            for _, event in ipairs(events) do
                print("事件发生:")
                print("  名称:", event.name)
                print("  掩码:", event.mask)
                print("  Cookie:", event.cookie) -- 用于重命名事件
                print("  监视描述符:", event.wd) -- 触发事件的监视器的描述符
                print("  标志:")
                if event.create then print("    CREATE") end
                if event.delete then print("    DELETE") end
                if event.modify then print("    MODIFY") end
                -- ... 等等其他事件类型
            end
        else
            print("没有事件发生。") -- 对于非阻塞读取
        end
    end)
end)