local tablex = require "pl.tablex"
local skynet = require "skynet"
local enum = require "common.enum"
local OPERATIONS = enum.new({
    INSERT = "insert",
    SET = "set",
    UPDATE = "update",
    DELETE = "delete",
    CLEAR = "clear",
})

local OPERATION_FUNC = {
    [OPERATIONS.INSERT] = function(self, prev, current, paths, value)
        if type(current) ~= "table" then
            error("Cannot insert into non-table at path: " .. table.concat(paths, "."))
        end
        table.insert(current, tablex.deepcopy(value))
        self:_triggerUpdate(OPERATIONS.INSERT, paths, #current, value)
    end,
    [OPERATIONS.SET] = function(self, prev, current, paths, k, v)
        if type(current) ~= "table" then
            error("Cannot add into non-table at path: " .. table.concat(paths, "."))
        end

        current[k] = v
        self:_triggerUpdate(OPERATIONS.SET, paths, k, v)
    end,
    [OPERATIONS.UPDATE] = function(self, prev, current, paths, value)
        if type(value) ~= "table" then
            error("Cannot update using non-table at path: " .. table.concat(paths, "."))
        end
        prev[paths[#paths]] = tablex.deepcopy(value)
        self:_triggerUpdate(OPERATIONS.UPDATE, paths, value)
    end,
    [OPERATIONS.DELETE] = function(self, prev, current, paths, ...)
        prev[paths[#paths]] = nil
        self:_triggerUpdate(OPERATIONS.DELETE, paths)
    end,
    [OPERATIONS.CLEAR] = function(self, prev, current, paths, ...)
        tablex.clear(current)
        self:_triggerUpdate(OPERATIONS.CLEAR, paths)
    end,
}

local function createProxy(manager, paths)
    local proxy = {}
    local mt = {
        __index = function(t, key)
            -- 方法调用拦截
            local op = OPERATIONS:enum(key)
            if op then
                return function(value, ...)
                    manager:_performOperation(op, paths, value, ...)
                end
            end

            local rawv = manager:_getRawDataByPath(paths)
            if not rawv then --or not rawv[key] then
                return
            end

            local v = rawv[key]
            if not v or type(v) ~= "table" then
                return v
            end

            -- print("__index", t, key)
            -- 路径追踪
            local newPath = tablex.deepcopy(paths)
            table.insert(newPath, key)
            return createProxy(manager, newPath)
        end,

        __newindex = function()
            error("Direct assignment is prohibited, use update() method instead")
        end,
        -- 新增遍历支持
        __pairs = function(t)
            local rawData = manager:_getRawDataByPath(paths)
            if type(rawData) ~= "table" then
                error("Cannot iterate non-table value at path: " .. table.concat(paths, "."))
            end

            local function stateless_iter(tbl, k)
                local nk, nv = next(tbl, k)
                if nk then
                    -- 创建子代理对象
                    if type(nv) ~= "table" then
                        return nk, nv
                    end

                    local childPath = table.pack(table.unpack(paths))
                    table.insert(childPath, nk)
                    return nk, createProxy(manager, childPath)
                end
            end

            return stateless_iter, rawData, nil
        end,

        __len = function()
            local rawData = manager:_getRawDataByPath(paths)
            return #rawData
        end
    }
    return setmetatable(proxy, mt)
end

local DataManager = {}
DataManager.__index = DataManager

function DataManager.new(initData, version, onChangeCallback)
    local self = setmetatable({
        _data = tablex.deepcopy(initData or {}),
        _version = version,
        _version_track = version,
        _callbacks = { onChangeCallback },
        _proxy = nil,
    }, DataManager)

    local publicMethods = {
        raw = function()
            return self._data
        end,

        clone = function()
            return tablex.deepcopy(self._data)
        end,

        isDirty = function()
            return self._version ~= self._version_track
        end,

        getVersion = function()
            return self._version
        end,

        setVersion = function(version_track)
            self._version_track = version_track
        end,

        resetVersion = function(_version)
            if _version then
                self._version = _version
                return
            end

            self._version = 1
        end,

        dump = function(...)
            local rets = {}
            local dumpall = select("#", ...)

            if dumpall then
                for name, module in pairs(self._data) do
                    if name ~= "uid" and name ~= "id" then
                        table.insert(rets, name)
                        table.insert(rets, skynet.packstring(module))
                    end
                end
            else
                for _, name in ipairs({...}) do
                    table.insert(rets, name)
                    table.insert(rets, skynet.packstring(self._data[name] or {}))
                end
            end

            return self._version, table.unpack(rets)
        end,

        addListener = function(callback)
            table.insert(self._callbacks, callback)
        end,

        removeListener = function(callback)
            tablex.removeValues(callback)
        end
    }

    -- 创建根代理对象
    self._proxy = createProxy(self, {})

    return setmetatable(publicMethods, {
        __index = function(_, k)
            if k == "data" then
                error("Direct access to 'data' is prohibited, use get() method")
            end
            return self._proxy[k] -- 返回代理对象的方法
        end,

        __newindex = function()
            error("Modification prohibited")
        end,

        -- 新增遍历支持
        __pairs = function(t)
            local function stateless_iter(tbl, k)
                local nk, nv = next(tbl, k)
                if nk then
                    -- 创建子代理对象
                    if type(nv) ~= "table" then
                        return nk, nv
                    end

                    return nk, createProxy(self, { nk })
                end
            end

            return stateless_iter, self._data, nil
        end,

        __len = function()
            return #self._data
        end,
    })
end


function DataManager.loadplayer(t, callback)
    if not t or t.errno or t.err then return end

    local uid = t.uid
    local version = t.version
    if not uid or not version then
        return
    end

    local player = {}
    for k, v in pairs(t) do
        player[k] = type(v) == "string" and skynet.unpack(v) or v
    end

    return DataManager.new(player, version, callback)
end

function DataManager:_performOperation(opType, paths, ...)
    local current, prev = self:_getRawDataByPath(paths)

    -- 执行操作
    local opfunc = OPERATION_FUNC[opType]
    if opfunc then
        opfunc(self, prev, current, paths, ...)
    end
end

-- 在DataManager类中添加内部方法
function DataManager:_getRawDataByPath(paths)
    local current = self._data
    local prev = current
    for _, seg in ipairs(paths) do
        if type(current) ~= "table" then return nil end
        prev = current
        current = current[seg]
    end
    return current, prev
end

function DataManager:_triggerUpdate(opType, paths, ...)
    self._version = self._version + 1
    for _, callback in ipairs(self._callbacks) do
        if type(callback) == 'function' then
            callback({
                version = self._version,
                action = OPERATIONS:upperValue(opType),
                paths = paths,
                -- timestamp = os.time(),
                params = { ... },
            })
        end
    end
end

return DataManager
