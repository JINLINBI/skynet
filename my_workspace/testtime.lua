local skynet = require "skynet"

function task()
    skynet.error("task")
    skynet.error("start time", skynet.starttime())
    skynet.sleep(200)
    skynet.error("time", skynet.time())
    skynet.error("now", skynet.now())
end

skynet.start(function()
    skynet.fork(task)
end)
