local settings = {}

settings.debug = true

settings.appname = "app" --app name

settings.word_crab_file = '../common/word_crab/words.txt'

settings.protocol_type = "json" --msgpack or json

settings.save_policy = "async"  --sync or async

settings.mini_conf = {
    appid  = "",
    secret = "",
}

settings.h5_conf = {
    appid  = "",
    secret = "",
}

settings.mch_conf = {
    mch_id = "",
    key    = "",
}

settings.wx_notify_url = ""

-- 登陆认证服
settings.login_conf = {
    node_name         = "loginserver",
    console_port      = 10800,
    login_port_http   = 8002, --(暴露) 登陆认证端口
    login_port_tcp    = 8001, --(暴露) 登录认证端口
    login_slave_count = 8,    --登陆认证代理个数
    api_server_ca     = "hDJ^54D@!&DH*Sdh18(Sahjig123",
    max_client        = 6000
}


-- 游戏服务配置
settings.nodes = {

    gameserver101 = {
        -- 网络配置
        server_no       = 1,             --服务器编号
        node_name       = "gameserver101", --
        console_port    = 10801,         --
        host            = "0.0.0.0",     -- 需要手动修改
        gate_port_tcp   = 8888,          --(暴露 网关端口 TCP)
        gate_port_ws    = 8889,          --(暴露 网关端口 WS)
        gate_port_http  = 11001,
        max_client      = 6000,
        nodelay         = true,
        api_slave_count = 10,
        dbproxy = {"mysqldb", "redisdb"},
        redisdb_maxinst = 10,
        redisdb_cnf = {
            host = "localhost",
            port = 6379,
            db = 0,
        },
        mysqldb_maxinst = 10,
        mysqldb_cnf = {
            ip = "localhost",
            port = 3306,
            user = "root",
            password = "123456",
            db = "skynet"
        },
    },

    gameserver102 = {
        -- 网络配置
        server_no       = 2,             --服务器编号
        node_name       = "gameserver102", --
        console_port    = 10801,         --
        host            = "0.0.0.0",     -- 需要手动修改
        gate_port_tcp   = 8888,          --(暴露 网关端口 TCP)
        gate_port_ws    = 8889,          --(暴露 网关端口 WS)
        gate_port_http  = 11001,
        max_client      = 6000,
        nodelay         = true,
        api_slave_count = 10,
        dbproxy = {"mysqldb", "redisdb"},
        redisdb_maxinst = 10,
        redisdb_cnf = {
            host = "localhost",
            port = 6379,
            db = 0,
        },
        mysqldb_maxinst = 10,
        mysqldb_cnf = {
            ip = "localhost",
            port = 3306,
            user = "root",
            password = "123456",
            db = "skynet"
        },
    },
}


return settings
