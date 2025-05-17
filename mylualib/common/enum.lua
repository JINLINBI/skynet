-- Enum class definition
EnumClass = EnumClass or {}
EnumClass.__index = EnumClass

-- Constructor
function EnumClass.new(nameTable, defaultName)
    local self = setmetatable({}, EnumClass)
    self._nameTable = nameTable
    self._reverseNameTable = {}
    self._upperNameTable = {}
    self._lowerNameTable = {}
    self._defaultName = defaultName or "unknown"
    self._defaultUpperName = string.upper(defaultName or "UNKNOWN")
    self._defaultLowerName = string.lower(defaultName or "unknown")
    self._idx2k = {}

    local ks = {}
    for k, v in pairs(nameTable) do
        self._reverseNameTable[v] = k
        self._upperNameTable[k] = string.upper(v)
        self._reverseNameTable[k] = string.lower(v)
        table.insert(ks, k)
    end

    table.sort(ks)
    for idx, k in ipairs(ks) do
        self[k] = idx
        self._idx2k[idx] = k
    end
    return self
end

-- Function to get type from name
function EnumClass:key(v)
    return self._reverseNameTable[v]
end

function EnumClass:enum(v)
    local k = self._reverseNameTable[v]
    if not k then return end

    return self[k]
end

-- Function to get name from type
function EnumClass:value(k_or_enum)
    local k = self._idx2k[k_or_enum]
    if not k then k = k_or_enum end
    return self._nameTable[k_or_enum] or self._defaultName
end

function EnumClass:upperValue(k_or_enum)
    local k = self._idx2k[k_or_enum]
    if not k then k = k_or_enum end
    return self._upperNameTable[k] or self._defaultUpperName
end

function EnumClass:lowerValue(k_or_enum)
    local k = self._idx2k[k_or_enum]
    if not k then k = k_or_enum end
    return self._lowerNameTable[k] or self._defaultLowerName
end

return EnumClass
