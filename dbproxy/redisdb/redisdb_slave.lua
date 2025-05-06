local skynet = require "skynet.manager"
local redis = require "skynet.db.redis"
local inspect = require "inspect"


local CMD = {}
local db = nil


function CMD.start(cnf)
    -- DEBUG("redis slave cnf = ", inspect(cnf))
    local ok, d = pcall(redis.connect, cnf)
    if ok then
        db = d
        -- db:flushall()
    else
        ERROR("---redis connect error---", inspect(cnf))
    end
    skynet.fork(function()
        while true do
            local ping = db:ping()
            -- DEBUG("ping = ", ping)
            skynet.sleep(1000)
        end
    end)
end

function CMD.set(key, value, type, time)
    if type ~= nil then
        return db:set(key, value, type, time)
    else
        return db:set(key, value)
    end
end

function CMD.expire(key, ex)
    return db:expire(key, ex)
end

function CMD.get(key)
    return db:get(key)
end

function CMD.hmset(key, t)
    local data = {}
    for k, v in pairs(t) do
        table.insert(data, k)
        table.insert(data, v)
    end
    return db:hmset(key, table.unpack(data))
end

function CMD.hmget(key, ...)
    return db:hmget(key, ...)
end

function CMD.hset(key, filed, value)
    return db:hset(key, filed, value)
end

function CMD.hget(key, filed)
    return db:hget(key, filed)
end

function CMD.hgetall(key)
    return db:hgetall(key)
end

function CMD.zadd(...)
    return db:zadd(...)
end

function CMD.keys(key)
    return db:keys(key)
end

function CMD.zrange(key, from, to, scores)
    if not scores then
        return db:zrange(key, from, to)
    else
        return db:zrange(key, from, to, scores)
    end
end

function CMD.zincrby(key, score, member)
    return db:zincrby(key, score, member)
end

function CMD.zrevrange(key, from, to, scores)
    if not scores then
        return db:zrevrange(key, from, to)
    else
        return db:zrevrange(key, from, to, scores)
    end
end

function CMD.zrangebyscore(key, from, to, scores, limit, offset, count)
    if not scores then
        if not limit then
            return db:zrangebyscore(key, from, to)
        else
            return db:zrangebyscore(key, from, to, limit, offset, count)
        end
    else
        if not limit then
            return db:zrangebyscore(key, from, to, scores)
        else
            return db:zrangebyscore(key, from, to, scores, limit, offset, count)
        end
    end
end

function CMD.zrevrangebyscore(key, max, min, scores, limit, offset, count)
    if not scores then
        if not limit then
            return db:zrevrangebyscore(key, max, min)
        else
            return db:zrevrangebyscore(key, max, min, limit, offset, count)
        end
    else
        if not limit then
            return db:zrevrangebyscore(key, max, min, scores)
        else
            return db:zrevrangebyscore(key, max, min, scores, limit, offset, count)
        end
    end
end

function CMD.zrank(key, member)
    return db:zrank(key, member)
end

function CMD.zrevrank(key, member)
    return db:zrevrank(key, member)
end

function CMD.zscore(key, member)
    return db:zscore(key, member)
end

function CMD.zcount(key, from, to)
    return db:zcount(key, from, to)
end

function CMD.zcard(key)
    return db:zcard(key)
end

function CMD.incr(key)
    return db:incr(key)
end

function CMD.del(key)
    return db:del(key)
end

function CMD.hexists(key)
    local r = db:hexists(key)
    return r == 1 and true or false
end

function CMD.exists(key)
    return db:exists(key) == 1
end

function CMD.hdel(...)
    return db:hdel(...)
end

function CMD.hincrby(key, field, increment)
    return tonumber(db:hincrby(key, field, increment))
end

function CMD.incrby(key, increment)
    return tonumber(db:incrby(key, increment))
end

function CMD.setnx(key, value)
    return db:setnx(key, value) == 1
end

function CMD.hsetnx(key, field, value)
    return db:hsetnx(key, field, value) == 1
end

function CMD.hkeys(key)
    return db:hkeys(key)
end

function CMD.getbit(key, offset)
    return db:getbit(key, offset)
end

function CMD.setbit(key, offset, value)
    return db:setbit(key, offset, value)
end

function CMD.eval(...)
    return db:eval(...)
end

skynet.start(function()
    skynet.dispatch("lua", function(_, _, cmd, ...)
        -- DEBUG("redis slave cmd = ", cmd)
        local f = assert(CMD[cmd], cmd .. " not found")
        skynet.retpack(f(...))
    end)
end)
