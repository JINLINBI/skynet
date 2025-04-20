local skynet = require "skynet.manager"



skynet.start(function()
    local version = require "version"

    -- 初始化 tbconfig 版本号（可选）
    if version.get("tbconfig") == 0 then
        log_info("[testversion main] [tbconfig] update", version.update("tbconfig")) -- 初始值为1
    end

    -- 并发更新测试
    for i = 1, 10000000 do
        version.update("tbconfig")
    end

    -- -- 验证结果（应输出101）
    skynet.fork(function()
        log_info("[testversion fork] start", skynet.time())
        for i = 1, 10000000 do
            version.update("tbconfig")
        end
        log_info("[testversion fork] tbconfig get", version.get("tbconfig"), "end", skynet.time())
        log_info("[testversion fork] tbconfig update 102400", version.update("tbconfig", 102400), "end", skynet.time())
        log_info("[testversion fork] tbconfig get", version.update("tbconfig", 102400), "end", skynet.time())
    end)
    log_info("[testversion main] tbconfig get", version.get("tbconfig"))
    -- skynet.exit()
end)
