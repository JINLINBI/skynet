local tablex = require "pl.tablex"
local PlayerData = {}

local _watch_mt = {
    -- 注册监听回调函数
    add_listener = function(self, callback)
        table.insert(self._listeners, callback)
    end,

    -- 触发数据变化事件
    _fire_event = function(self, path, action, key, value)
        for _, callback in ipairs(self._listeners) do
            callback(path, action, key, value)
        end
    end,

    -- 递归设置元表
    _wrap_table = function(self, raw_table, parent_path)
        if self.protected_tables[raw_table] then
            return self.protected_tables[raw_table]
        end

        -- 初始化时遍历现有表结构
        for k, v in pairs(raw_table) do
            if type(v) == "table" then
                -- 递归包装嵌套表
                raw_table[k] = self:_wrap_table(v, parent_path and (parent_path .. "." .. k) or k)
            end
        end

        -- ...其他代码...
        local wrapped_table = setmetatable({}, {
            __index = function(t, k)
                return raw_table[k] or self[k]
            end,

            __newindex = function(t, k, v)
                local current_path = parent_path and (parent_path .. "." .. k) or k

                -- 旧值处理
                local old_val = raw_table[k]
                if type(old_val) == "table" then
                    self:_fire_event(parent_path, "DELETE", k, old_val)
                    if v == nil then goto out end
                end

                -- 新值处理
                if type(v) == "table" then -- 嵌套表处理
                    self:_fire_event(parent_path, "ADD", k, v)
                    raw_table[k] = self:_wrap_table(v, current_path)
                elseif v == nil then
                    self:_fire_event(parent_path, "DELETE", k)
                else
                    self:_fire_event(parent_path, "SET", k, v)
                end

                ::out::
                raw_table[k] = v
            end,

            __pairs = function()
                return pairs(raw_table)
            end,

            __len = function(t)
                return #raw_table
            end
        })
        self.protected_tables[raw_table] = wrapped_table
        return wrapped_table
    end,
}

-- 创建可监听的数据结构
function PlayerData.new(o)
    local data = { _listeners = {}, protected_tables = {} }
    return setmetatable(data, {
        __index = _watch_mt,
        __newindex = function(t, k, v)
            _watch_mt.__newindex(t, k, v) -- 调用元方法
        end
    }):_wrap_table(o or {}, "root")
end

return PlayerData
