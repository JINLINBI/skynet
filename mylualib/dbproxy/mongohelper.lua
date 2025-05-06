local M = {}

local mongodbx = require "dbproxy.mongodbx"

function M.find(...)
    return mongodbx.find(...)
end

function M.findOne(...)
    return mongodbx.findOne(...)
end

function M.aggregate(...)
    return mongodbx.aggregate(...)
end

function M.upsert(...)
    return mongodbx.upsert(...)
end

function M.updateMany(...)
    return mongodbx.updateMany(...)
end

function M.insert(...)
    return mongodbx.insert(...)
end

function M.batch_insert(...)
    return mongodbx.batch_insert(...)
end

function M.del(...)
    return mongodbx.del(...)
end

function M.fetch_all(...)
    return mongodbx.fetch_all(...)
end

function M.exec(cmd, ...)
    return mongodbx.exec(cmd, ...)
end

return M
