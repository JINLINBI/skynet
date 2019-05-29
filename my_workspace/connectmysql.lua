local skynet = require 'skynet'
local mysql = require 'skynet.db.mysql'

local function dump(res, tab)
     tab = tab or 0
     if(tab == 0) then
        skynet.error("............dump...........")
    end
    if type(res) == "table" then
        skynet.error(string.rep("\t", tab).."{")
        for k,v in pairs(res) do
            if type(v) == "table" then
                dump(v, tab + 1)
            else
                skynet.error(string.rep("\t", tab), k, "=", v, ",")
            end
        end
        skynet.error(string.rep("\t", tab).."}")
    else
        skynet.error(string.rep("\t", tab) , res)
    end
end


skynet.start(function()
    local function on_connect(db)
        skynet.error("on_connect")
    end

    local db = mysql.connect({
        host = '127.0.0.1',
        port = 3306,
        database = 'test',
        user = 'jinlinbi',
        password = 'password',
        max_packet_size = 1024 * 1024,
        on_connect = on_connect
    })

    if not db then
        skynet.error("failed to connect")
    else
        skynet.error("success to connect to mysql server")
    end

    res = db:query("drop table if exists dogs")
    dump(res)


    --创建数据表
    res = db:query("create table dogs (id int primary key,name varchar(10))")
    dump(res)

    --语句查询
    res = db:query("select * from dogs")
    dump(res)

    --插入数据
    res = db:query("insert into dogs values (1, \'black\'), (2, \'red\')")
    dump(res)

    --插入数据
    res = db:query("insert into dogs values (3, \'blackjuice\'), (4, \'redpacet\')")
    dump(res)

    --多条语句查询
    res = db:query("select * from dogs")
    dump(res)

    if not res.err then
        for k, v in pairs(res) do
            skynet.error("res ... dogs.name ... " ..res[k].name)
        end
    end


    --查询错误
    res = db:query("select * from noexist;")
    dump(res)

    --关闭连接
    db:disconnect() 
    skynet.exit()

end)
