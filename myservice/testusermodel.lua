local skynet = require "skynet.manager"
local snowflake = require "snowflake"
local mysqlhelper = require "mysqlhelper"
local inspect = require "inspect"
local datamanager = require "data_manager"



skynet.start(function()
    local uid = snowflake.id()
    local player = datamanager.new({
        uid = uid,
        base = {
            uid = uid,
            name = "player1",
            level = 1,
        },
        items = {
        },
    }, 1)

    player.set("social", { friends = { { name = "player2", uid = snowflake.id() } } })

    log_info("player is dirty: ", player.isDirty())
    player.base.set("name", "playernewname")
    log_info("player is dirty: ", player.isDirty())

    for k, v in ipairs({ player.dump() }) do
        log_info(k, v)
    end
    local ret = mysqlhelper.newuser(uid, player.dump())

    log_info(inspect.inspect(ret))

    local ret = mysqlhelper.loaduser(player.uid)
    mysqlhelper.assertError(ret)
    log_info(inspect.inspect(ret))
    local playernew = datamanager.loadplayer(ret[1])
    log_info(inspect.inspect(playernew.clone()))

    for k, v in ipairs({player.dump()}) do
        log_info(k, v)
    end
    local saveret = mysqlhelper.saveuser(player.uid, player.dump())
    log_info(inspect.inspect(saveret))
end)
