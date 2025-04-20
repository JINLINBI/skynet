local M  = {}

function M.count(t)
    local count = 0
    for _, v in pairs(t) do
        print(_, v)
        count = count + 1
    end

    return count
end

return M