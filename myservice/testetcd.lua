local skynet = require "skynet"
-- local etcd = require "etcd"
local etcd = require "etcd.etcd"
local inspect = require "inspect"

skynet.start(function()
    local etcdCli, err = etcd.new({
        http_host = "http://localhost:2379",
        -- user = user,
        -- password = password,
        protocol = "v3",
        serializer ="json",  -- 默认使用json格式配置
        ttl = 3,
    })

    if not etcdCli then
        log_error("etcdd init wrong, ", err)
        return
    end

    local body = etcdCli:grant(10).body
    local ID = body.ID
    local TTL = body.TTL
    log_info("TTL", TTL)
    log_info("ID", ID)
    log_info("set hellworld", inspect(etcdCli:set("/skynet/node1", {helloworld = "niviaiiaiddkdjj"}, {lease = ID})))


    -- log_info("get /skynet/node1", inspect(etcdCli:get("/skynet/node1")))
    -- log_info("get /skynet/node2", inspect(etcdCli:get("/skynet/node2")))
    -- log_info("set hellworld", inspect(etcdCli:set("/skynet/node2", {helloworld = "不哈黄河a但凡扥经济法"})))
    -- log_info("get /skynet/node1", inspect(etcdCli:get("/skynet/node1")))
    -- log_info("get /skynet/node2", inspect(etcdCli:get("/skynet/node2")))
    -- log_info("delete /skynet/node1", inspect(etcdCli:delete("/skynet/node1")))
    -- log_info("delete /skynet/node2", inspect(etcdCli:delete("/skynet/node2")))
    -- log_info("get /skynet/node1", inspect(etcdCli:get("/skynet/node1")))
    -- log_info("get /skynet/node2", inspect(etcdCli:get("/skynet/node2")))
    -- log_info("get /skynet/node", inspect(etcdCli:setx("/skynet/node", {hello = "world"}, {timeout = 3})))
end)
