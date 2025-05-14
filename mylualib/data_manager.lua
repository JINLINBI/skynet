local tablex = require "pl.tablex"
local OPERATIONS = {
    INSERT = 1,
    SET = 2,
    UPDATE = 3,
    DELETE = 4,
    CLEAR = 5,
}

local function createProxy(manager, pathSegments)
    local proxy = {}
    local mt = {
        __index = function(t, key)
            -- 方法调用拦截
            if key == "insert" then
                return function(value)
                    manager:_performOperation(OPERATIONS.INSERT, pathSegments, value)
                end
            elseif key == "set" then
                return function(k, v)
                    manager:_performOperation(OPERATIONS.SET, pathSegments, k, v)
                end
            elseif key == "update" then
                return function(value)
                    manager:_performOperation(OPERATIONS.UPDATE, pathSegments, value)
                end
            elseif key == "delete" then
                return function()
                    manager:_performOperation(OPERATIONS.DELETE, pathSegments)
                end
            elseif key == "clear" then
                return function()
                    manager:_performOperation(OPERATIONS.CLEAR, pathSegments)
                end
            end

            local rawv = manager:_getRawDataByPath(pathSegments)
            if not rawv then --or not rawv[key] then
                return
            end

            local v = rawv[key]
            if not v or type(v) ~= "table" then
                return v
            end

            -- print("__index", t, key)
            -- 路径追踪
            local newPath = tablex.deepcopy(pathSegments)
            table.insert(newPath, key)
            return createProxy(manager, newPath)
        end,

        __newindex = function()
            error("Direct assignment is prohibited, use update() method instead")
        end,
        -- 新增遍历支持
        __pairs = function(t)
            local rawData = manager:_getRawDataByPath(pathSegments)
            if type(rawData) ~= "table" then
                error("Cannot iterate non-table value at path: " .. table.concat(pathSegments, "."))
            end

            local function stateless_iter(tbl, k)
                local nk, nv = next(tbl, k)
                if nk then
                    -- 创建子代理对象
                    if nv and type(nv) ~= "table" then
                        return nk, nv
                    end

                    local childPath = {}
                    for _, v in ipairs(pathSegments) do table.insert(childPath, v) end
                    table.insert(childPath, nk)
                    return nk, createProxy(manager, childPath)
                end
            end

            return stateless_iter, rawData, nil
        end,

        __len = function ()
            local rawData = manager:_getRawDataByPath(pathSegments)
            return #rawData
        end

    }
    return setmetatable(proxy, mt)
end

local DataManager = {}
DataManager.__index = DataManager

function DataManager.new(initData, onChangeCallback)
    local self = setmetatable({
        _data = tablex.deepcopy(initData or {}),
        _version = 1,
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

        getVersion = function()
            return self._version
        end,

        setVersion = function(version)
            self._version = version
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
        end
    })
end

function DataManager:_performOperation(opType, pathSegments, value, value2)
    local current = self._data
    local prev = current
    local path = {}
    local param = nil

    -- 遍历路径并验证
    for _, seg in ipairs(pathSegments) do
        table.insert(path, seg)
        if type(current) ~= "table" then
            error("Invalid path: " .. table.concat(path, "."))
        end
        prev = current
        current = current[seg]
    end

    -- 执行操作
    if opType == OPERATIONS.INSERT then
        if type(current) ~= "table" then
            error("Cannot insert into non-table at path: " .. table.concat(path, "."))
        end
        table.insert(current, tablex.deepcopy(value))
    elseif opType == OPERATIONS.SET then
        if type(current) ~= "table" then
            error("Cannot add into non-table at path: " .. table.concat(path, "."))
        end

        current[value] = value2
        param = value
    elseif opType == OPERATIONS.UPDATE then
        prev[path[#path]] = tablex.deepcopy(value)
    elseif opType == OPERATIONS.DELETE then
        prev[path[#path]] = nil
    elseif opType == OPERATIONS.CLEAR then

    end

    self:_triggerUpdate(opType, pathSegments, param)
end

-- 在DataManager类中添加内部方法
function DataManager:_getRawDataByPath(pathSegments)
    local current = self._data
    for _, seg in ipairs(pathSegments) do
        if type(current) ~= "table" then return nil end
        current = current[seg]
    end
    return current
end

local actionMap = {
    [OPERATIONS.INSERT] = "INSERT",
    [OPERATIONS.UPDATE] = "UPDATE",
    [OPERATIONS.DELETE] = "DELETE",
    [OPERATIONS.SET] = "SET",
}

function DataManager:_triggerUpdate(opType, path, param)
    self._version = self._version + 1
    for _, callback in ipairs(self._callbacks) do
        if type(callback) == 'function' then
            callback({
                version = self._version,
                action = actionMap[opType],
                path = path,
                timestamp = os.time(),
                param = param,
            })
        end
    end
end

return DataManager
