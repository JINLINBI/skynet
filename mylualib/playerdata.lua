local tablex = require "pl.tablex"
local types = require "pl.types"
local inspect = require "inspect"
local PlayerData = {}

local _watch_mt = {
    add_listener = function(self, callback)
        table.insert(self._listeners, callback)
    end,

    getVersion = function (self)
        return self._version
    end,

    -- 表操作系列方法
    clone = function(self)
        return tablex.deepcopy(self)
    end,

    insert = function (self, val)
        if not types.is_writeable(self) or not types.is_iterable(self) then
            log_error("self is not writeable", self)
            return
        end

        table.insert(self, val)
        return val
    end,

    remove = function (self, pos)
        pos = pos or #self
        log_error(inspect.inspect(self))
        log_error("remove self", self, pos, type(pos))
        local oldval = self[pos]
        self[pos] = nil
        return oldval
    end,

    clear = function (self, istart)
        tablex.clear(self, istart)
    end,


    -- 事件触发增加版本号追踪
    _fire_event = function(self, path, action, key, value)
        rawset(self, "_version", rawget(self, "_version") + 1)
        for _, cb in ipairs(self._listeners) do
            cb(path, action, key, value, self._version)
        end
    end,

    -- 递归包装（基于深拷贝数据）
    _wrap_table = function(insmt, raw_table, parent_path)
        -- 执行深度拷贝
        local copied_table = tablex.deepcopy(raw_table)

        local function recursive_wrap(tbl, current_path)
            setmetatable(tbl, {
                __index = function(t, k)
                    log_info("__index", k)
                    return rawget(t, k) or insmt[k]
                end,

                __newindex = function(t, k, v)
                    log_error("__newindex", t, k, v)
                    local old_val = rawget(t, k)
                    local event_path = current_path .. "." .. k

                    -- 旧值清理
                    if type(old_val) == "table" then
                        insmt:_fire_event(current_path, "DELETE", k, old_val)
                        goto out
                    end

                    -- 新值处理
                    if v ~= nil then
                        if type(v) == "table" then
                            v = insmt:_wrap_table(v, event_path)
                            insmt:_fire_event(current_path, "ADD", k, v)
                        else
                            insmt:_fire_event(current_path, "SET", k, v)
                        end
                    else
                        insmt:_fire_event(current_path, "DELETE", k)
                    end

                    ::out::
                    -- log_info("rawset", t, k, v)
                    rawset(t, k, v)
                end,
            })

            -- 递归处理子表
            for key, val in pairs(tbl) do
                if type(val) == "table" then
                    recursive_wrap(val, current_path .. "." .. key)
                end
            end
        end

        recursive_wrap(copied_table, parent_path or "root")
        return copied_table
    end
}

-- 创建可监听的数据结构
function PlayerData.new(o, version)
    local insmt = {
        _version = version or 0,
        _listeners = {},
        _data_root = nil,
    }

    setmetatable(insmt, { __index = _watch_mt })

    -- 初始化深拷贝并包装
    return insmt:_wrap_table(o or {}, "root")
end

return PlayerData
