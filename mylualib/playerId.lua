local skynet = require "skynet"

local M = {}
local wokerId = 0
local wokerReady = 0
local nowTs = skynet.time() * 10
local now = (math.floor(nowTs * 10) & (2^42 - 1)) << 12
local sequence = 0

-- 初始化机器码（结合MAC地址与进程ID）
local function initWorkderId()
    -- 获取本机MAC地址末两段（Skynet环境适配）
    local mac = skynet.getenv("mac_address") or "00:00:00:00:00:00"
    local segments = {}
    for v in mac:gmatch("%x+") do
        table.insert(segments, tonumber(v, 16))
    end

    -- 生成10位机器码[3,10](@ref)
    local len = #segments
    wokerId = (((segments[len-1] | (2^5 - 1)) << 8) | segments[len]) % 1024
    wokerReady = wokerId << 53
end

-- 生成下一个ID（线程安全）
function M.nextId()
    sequence = sequence + 1

    -- 组合ID（Lua需用53位精度处理）
    local id = wokerReady | (now + sequence)
    return id
end

-- 数字转换成62进制代表的短字符串
local charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
function M.id2ShortStr(num)
    local base = #charset  -- 62 characters in the charset
    local result = {}

    -- Handle zero explicitly
    if num == 0 then
        return charset:sub(1, 1)  -- '0'
    end

    -- Convert the number to base 62
    while num > 0 do
        local remainder = (num % base) + 1  -- Lua index starts at 1
        table.insert(result, 1, charset:sub(remainder, remainder))
        num = num // base
    end

    -- Combine the table into a single string
    return table.concat(result)
end

function M.shortStr2Id(str)
    local base = #charset  -- 62 characters in the charset
    local num = 0

    -- Create a lookup table to map characters to their positions in the charset
    local char_to_index = {}
    for i = 1, base do
        char_to_index[charset:sub(i, i)] = i - 1  -- Store index as 0-based
    end

    -- Convert the short string back to a number
    for i = 1, #str do
        local char = str:sub(i, i)
        num = num * base + char_to_index[char]
    end

    return num
end

-- 解析ID
function M.parseId(id)
    return {
        machine = (id >> 53) % 1024,
        timestamp = (id << 11) >> (12 + 11),
        sequence = id % 4096
    }
end


initWorkderId()
return M