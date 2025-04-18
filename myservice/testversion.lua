local skynet = require "skynet"



skynet.start(function ()
    local version = require "version"

    -- 初始化 tbconfig 版本号（可选）
    if version.get("tbconfig") == 0 then
        version.update("tbconfig") -- 初始值为1
    end

    -- 并发更新测试
    for i=1, 10000000 do
        version.update("tbconfig")
    end

    -- -- 验证结果（应输出101）
    skynet.fork(function ()
        for i=1, 10000000 do
            version.update("tbconfig")
        end
        print("version  get", version.get("tbconfig"))
    end)
    print("version  get", version.get("tbconfig"))
end)