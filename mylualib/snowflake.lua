local skynet = require "skynet.manager"
local harbor = require "skynet.harbor"

local M = {}
local workerId = 0
local nowTs = skynet.time() * 10
local now = (math.floor(nowTs * 10) & (2^42 - 1)) << 12
local sequence = 0

-- 字符集缓存
local charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
local charset_len = #charset
local char_to_index = {}

-- 初始化字符映射表
for i = 1, charset_len do
    char_to_index[charset:sub(i, i)] = i - 1
end

-- 初始化机器码（结合MAC地址与进程ID）
local function initWorkderId()
    -- 获取本机MAC地址末两段（Skynet环境适配）
    local nodeId
    while not nodeId or nodeId == 0 do
        nodeId = tonumber(string.gmatch(skynet.getenv "node_name", "(%d+)")())
        if not nodeId or nodeId == 0 then
            log_error("load nodeId failed: config node_name invalid!", skynet.getenv "node_name")
        end
    end
    workerId = nodeId << 53
end

-- 生成下一个ID（线程安全）
function M.id()
    sequence = sequence + 1

    -- 组合ID（Lua需用53位精度处理）
    local id = workerId | (now + sequence)
    return id
end

-- 解析ID
function M.parseId(id)
    return {
        machine = (id >> 53) % 1024,
        timestamp = (id << 11) >> (12 + 11),
        sequence = id % 4096
    }
end

-- 数字转换成62进制代表的短字符串（优化版）
function M.id2ShortStr(num)
    -- Handle zero explicitly
    if num == 0 then
        return charset:sub(1, 1)  -- '0'
    end

    local result = {}
    -- Convert the number to base 62
    while num > 0 do
        local remainder = (num % charset_len) + 1  -- Lua index starts at 1
        table.insert(result, 1, charset:sub(remainder, remainder))
        num = num // charset_len
    end

    -- Combine the table into a single string
    return table.concat(result)
end

function M.shortStr2Id(str)
    local num = 0
    -- Convert the short string back to a number
    for i = 1, #str do
        local char = str:sub(i, i)
        num = num * charset_len + char_to_index[char]
    end

    return num
end


initWorkderId()
return M