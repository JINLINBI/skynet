local skynet = require "skynet"


function task()
    skynet.error("task", coroutine.running())
    skynet.timeout(500, task)
end


skynet.start(function() 
    skynet.error("start", coroutine.running())
    skynet.timeout(500, task)
end)
