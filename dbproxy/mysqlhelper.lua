local M = {}

local mysqldbx = require "mysqldbx"


function M.loaduser(uid)
    local sql = string.format("SELECT * FROM `t_role_data` WHERE uid = %d", uid)
    return mysqldbx.query(sql)
end

function M.saveuser(uid, player, model)
    
end

return M
