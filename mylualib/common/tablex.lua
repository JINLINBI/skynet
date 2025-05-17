local M = {}

-- 正向遍历列表
function M.traverse(t, func)
    for k, v in ipairs(t) do
        local bk, ret = func(v, k)
        if bk then return ret end
    end
end

-- 反向遍历表元素
function M.reTraverse(t, func)
    for k = #t, 1, -1 do
        local v = t[k]
        local bk, ret = func(v, k)
        if bk then return ret end
    end
end


-- 查找元素是否存在的函数
function M.find(t, value)
    for k, v in pairs(t) do
        if v == value then
            return k -- 返回找到的键或索引
        end
    end
    return nil -- 未找到返回 nil
end


-- 根据match函数返回值查找元素
function M.findMatch(t, matchFunc)
    for k, v in pairs(t) do
        local match, ret = matchFunc(v, k)
        -- 返回找到的元素或者match的第二个返回值
        if match then return v, ret end
    end
end

-- 根据值删除列表数据
function M.remove(t, item)
    for k, v in pairs(t) do
        if v == item then
            return table.remove(t, k)
        end
    end
end

-- 删除元素列表
function M.removeItems(t, items)
    for _, item in pairs(items) do
        M.remove(t, item)
    end
end

-- 根据match函数返回值删除元素
function M.removeMatch(t, matchFunc)
    for k, v in pairs(t) do
        local match, ret = matchFunc(v, k)
        if match then
            table.remove(t, k)
            return v, ret -- 返回找到的元素或者match的第二个返回值
        end
    end
end

-- 根据match函数返回值删除列表中的所有元素
function M.removeIf(t, matchFunc)
    for idx = #t, 1, -1 do
        local v = t[idx]
        local match = matchFunc(v)
        if match then table.remove(t, idx) end
    end
end


-- 合并两个列表（去重，不改变原数据）
function M.mergeList(t1, t2)
    if not t2 then return end

    local result = {}
    for _, v in ipairs(t1) do
        table.insert(result, v)
    end

    for _, v in ipairs(t2) do
        if not M.find(result, v) then
            table.insert(result, v)
        end
    end

    return result
end

-- 合并两个列表（不去重，改变原数据）
function M.appendList(t1, t2)
    if not t2 then return t1 end

    for _, v in ipairs(t2) do
        table.insert(t1, v)
    end

    return t1
end

-- 合并列表中相同的元素
---@param t table: 需要合并的列表
---@param isSameFunc function:M. isSameFunc(perItem, rmItem) 返回true即删除rmItem
function M.mergeSame(t, isSameFunc)
    if #t < 2 then return t end

    for rmIdx = #t, 2, -1 do
        local rmItem = t[rmIdx]
        for idx = 1, rmIdx - 1 do
            local preItem = t[idx]
            if isSameFunc(preItem, rmItem) then
                table.remove(t, rmIdx)
                break
            end
        end
    end

    return t
end


--- 可以计算非列表的表元素个数
---@param t table
function M.count(t)
    local count = 0
    for k in next, t, nil do count = count + 1 end
    return count
end


function M.clear(t)
    for k in ipairs(t) do
        t[k] = nil
    end
end


function M.transToIdx(t)
    local ret = {}
    for _, v in pairs(t) do
        ret[v] = true
    end

    return ret
end


return M