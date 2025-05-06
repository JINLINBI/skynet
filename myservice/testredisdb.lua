local skynet = require "skynet"
local redisdbx = require "redisdbx"


skynet.start(function()
    log_info("test redisdb start")
    local res = redisdbx.exec("get", 0, "helloworld")
    log_info("test redisdb: get helloworld ", res)
    local res = redisdbx.exec("set", 0, "helloworld", "hell!!!!")
    log_info("test redisdb: set helloworld ", res)
    local res = redisdbx.exec("get", 0, "helloworld")
    log_info("test redisdb: get helloworld ", res)
end)
