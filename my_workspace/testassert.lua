local skynet = require "skynet"

function task1()
    skynet.error("task1", coroutine.running())
    skynet.sleep(100)
    assert(nil)
    skynet.error("task2", coroutine.running(), "end")
end


function task2()
    skynet.error("task2", coroutine.running())
    skynet.sleep(500)
    skynet.error("task1", coroutine.running(), "end")
end


skynet.start(function()
    skynet.error("start", coroutine.running())
    skynet.fork(task1)
    skynet.fork(task2)
end)
