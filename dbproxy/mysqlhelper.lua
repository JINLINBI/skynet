local tablex = require "pl.tablex"
local mysqldbx = require "mysqldbx"
local M = {}

local function splitModelnameAndData(...)
    local fieldnames = {}
    local datas = {}

    local index = 1
    for _, v in ipairs({...}) do
        if index % 2 == 1 then
            table.insert(fieldnames, v)
        else
            table.insert(datas, v)
        end
        index = index + 1
    end

    return fieldnames, datas
end

function M.loaduser(uid)
    if not uid then
        log_error("loaduser failed: uid is nil")
        return { err = "Invalid uid" }
    end
    local prepare = string.format("SELECT * FROM `t_role_data` WHERE uid = ? LIMIT 1")
    return mysqldbx.execute(prepare, uid)
end

function M.saveuser(uid, version, modelname, modeldata, ...)
    if not uid then
        log_error("saveuser failed: uid is nil")
        return { err = "Invalid uid" }
    end
    if not version then
        log_error("saveuser failed: version is nil")
        return { err = "Invalid version" }
    end

    local len = select("#", modelname, modeldata, ...)
    if len == 0 or (len % 2 ~= 0) then
        log_error("saveuser failed: modelname not match modeldata", len)
        return { err = "Invalid parameters" }
    end

    local fieldnames, datas = splitModelnameAndData(modelname, modeldata, ...)
    table.insert(fieldnames, 1, "version")
    table.insert(datas, 1, version)
    local prepare = string.format("UPDATE `t_role_data` SET %s%s WHERE uid = ?",
        table.concat(fieldnames, " = ?, "), " = ?");

    log_debug("prepare statement", prepare)
    table.insert(datas, uid)
    local ret = mysqldbx.execute(prepare, table.unpack(datas))
    log_debug("save user ret", ret)
    return ret
end

function M.newuser(uid, version, modelname, modeldata, ...)
    if not uid then
        log_error("newuser failed: uid is nil")
        return { err = "Invalid uid" }
    end
    if not version then
        log_error("newuser failed: version is nil")
        return { err = "Invalid version" }
    end

    local len = select("#", modelname, modeldata, ...)
    if len == 0 or (len % 2 ~= 0) then
        log_error("newuser failed: modelname not match modeldata", len)
        return { err = "Invalid parameters" }
    end

    local fieldnames, datas = splitModelnameAndData(modelname, modeldata, ...)
    local questmarks = tablex.new(#datas, "?")
    local prepare = string.format("INSERT `t_role_data` (uid, version, %s) VALUES (?, ?, %s)",
        table.concat(fieldnames, ", "), table.concat(questmarks, ", "));

    log_debug("prepare statement", prepare)
    local ret = mysqldbx.execute(prepare, uid, version, table.unpack(datas))
    log_debug("new user ret", ret)
    return ret
end

function M.batchsave()

end


function M.assertError(ret)
    assert(ret.err == nil and ret.errno == nil)
end

return M
