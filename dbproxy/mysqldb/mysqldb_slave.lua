local skynet = require "skynet"
require "skynet.manager"
local mysql = require "skynet.db.mysql"

local CMD = {}
local stmts = {}
local db


function CMD.start(conf)
    local function on_connect(db)
        db:query("set charset utf8mb4");
    end

    db = mysql.connect {
        host = conf.ip,
        port = conf.port,
        database = conf.db,
        user = conf.user,
        password = conf.password,
        charset = "utf8mb4",
        max_packet_size = 1024 * 1024,
        on_connect = on_connect
    }
    if not db then
        error("mysql connect fail")
    end
    skynet.fork(function()
        while true do
            db:ping()
            skynet.sleep(300)
        end
    end)
end

function CMD.query(sql)
    return db:query(sql)
end

function CMD.execute(prepare, ...)
    local stmt = stmts[prepare] or db:prepare(prepare)
    if stmt and (not stmt.err or not stmt.errno) then
        stmts[prepare] = stmt
    end

    -- local stmt = db:prepare(prepare)

    local res = db:execute(stmt, ...)
    if res.errno and res.errno == 1243 then
        -- err = "Unknown prepared statement handler (3) given to mysqld_stmt_execute",
        -- errno = 1243,
        stmts[prepare] = nil
    end

    return res
end

function CMD.prepare(prepare, ...)
    return db:prepare(prepare, ...)
end

function CMD.executestmt(stmt, ...)
    return db:execute(stmt, ...)
end

skynet.start(function()
    skynet.dispatch("lua", function(_, _, cmd, ...)
        local f = assert(CMD[cmd], cmd .. "not found")
        skynet.retpack(f(...))
    end)
end)
