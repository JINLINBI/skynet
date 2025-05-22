local skynet = require "skynet"
local mysqldbx = require "mysqldbx"
local mysqldbhelper = require "mysqlhelper"
local inspect = require "inspect"

skynet.start(function()
    -- local dbserviceName = "dbservice"
    -- skynet.send(dbserviceName, "lua", "exec", "drop table if exists cats")
    -- skynet.send(dbserviceName, "lua", "exec", "create table cats "
    -- 	               .."(id serial primary key, ".. "name varchar(5))")
    -- skynet.send(dbserviceName, "lua", "exec", "drop table if exists cats")
    -- skynet.send(dbserviceName, "lua", "query", "select * from cats order by id asc")


    log_info("test luasql start")
    local res = mysqldbx.query("SELECT * FROM test;")
    local uid0 = 250010
    local baseblob = {
        a = 1,
        b = true,
        c = "bulll",
        d = { 1,2,3,4 },
        name = "helloworld",
        uid = uid0,
        version = 1,
    }

    local socialblob = {
        friends = {"helloworlda", "a", "b", "c"},
        b = true,
        c = "0",
        d = { 1,2,3,4 },
        name = "helloworld",
        uid = uid0,
        version = 1,
    }

    -- local res = mysqldbx.query(string.format("INSERT INTO t_role_data (uid, base, social) VALUES (%d, '%s', '%s');", uid, skynet.packstring(baseblob), mysql.quote_sql_str(skynet.packstring(socialblob))))
    -- log_info("test luasql: ret info ", inspect(res))


    log_info("skynet.pack", skynet.pack(baseblob))
    local start = skynet.now()
    log_info("start test at", start)
    for i = 1, 10000 do
        local uid = uid0 + i
        local res = mysqldbx.execute("INSERT `t_role_data` (uid, base, social) VALUES (?, ?, ?);", uid, 12223, skynet.packstring(socialblob))
        if res.err or res.errno then
            log_error("invalid execute", res.err)
            break
        end
        -- log_info("res", uid, inspect(res))

        -- local playerseri = mysqldbhelper.loaduser(uid)[1]
        -- -- log_info("player load raw", inspect(playerseri))
        -- local player = {
        --     uid = playerseri.uid,
        --     base = skynet.unpack(playerseri.base),
        --     social = skynet.unpack(playerseri.social),
        -- }
        -- log_info("player load un÷pack", inspect(player))
    end
    local endtime = skynet.now()
    log_info("end insert at", endtime, "using", endtime - start)

    -- local stmt = mysqldbx.prepare("INSERT t_role_data (uid, base, social) VALUES (?, ?, ?);")
    -- log_info("stmt handler", stmt, inspect(stmt))
    -- local res1 = mysqldbx.executestmt(stmt, uid, skynet.packstring(baseblob), skynet.packstring(socialblob))
    -- log_info("res1", inspect(res1))


    -- local playerseri = mysqldbhelper.loaduser(uid + 1)[1]
    -- log_info("player load  playerseri", inspect(playerseri))
    -- local player = {
    --     uid = playerseri.uid,
    --     base = skynet.unpack(playerseri.base),
    --     social = skynet.unpack(playerseri.social),
    -- }
    -- log_info("player load unpack", inspect(player))
end)
