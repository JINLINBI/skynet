local skynet = require "skynet"
local datamanager = require "data_manager"


skynet.start(function()
    local manager = datamanager.new({
        user = {
            profile = { name = "Alice" },
            items = {},
        }
    })

    manager.addListener(function(event)
        print(string.format("[v%d] %s at %s %s %s",
            event.version,
            event.action,
            table.concat(event.paths, "."),
            tostring(event.params[1]),
            event.params[2] and tostring(event.params[2]) or ""
        ))
    end)

    -- 链式操作示例
    manager.user.profile.update({ name = "Bob", age = 25, items = { "hello", "world", "again" } }) -- UPDATE at user.profile
    manager.user.profile.set("name", "China")                                                   -- UPDATE at user.profile
    -- manager.user.profile.name.delete()                       -- UPDATE at user.profile

    manager.user.items.insert("sword")            -- INSERT at user.items
    manager.user.items.insert("shield")           -- INSERT at user.items
    manager.user.items.insert("done't try again") -- INSERT at user.items
    manager.user.items.set("helloworld", "nihao shijie")
    manager.user.items.set("helloworld", "nihao shijie v2")
    print("manager.user.items.count", #manager.user.items)
    -- manager.user.items.delete()

    -- manager.user.items.delete()                             -- DELETE at user.items

    print("pairs manager.user.profile", manager.user.profile)
    for k, v in pairs(manager.user.profile) do
        print(k, v)
        if type(v) == "table" then
            for kk, vv in pairs(v) do
                print(kk, vv)
            end
        end
    end

    print("ipairs manager.user.profile", manager.user.profile)
    for k, v in ipairs(manager.user.profile) do
        print(k, v)
    end

    print("pairs manager.user.items", manager.user.items)
    for k, v in pairs(manager.user.items) do
        print(k, v)
    end

    print("ipairs manager.user.items", manager.user.items)
    for k, v in ipairs(manager.user.items) do
        print(k, v)
    end

    manager.user.items.delete()

    -- print("manager.user", manager.user)
    -- print("manager.user", table.insert(manager.user, "helloworld"))

    -- print("manager.user.profile", manager.user.profile)
    -- print("manager.user.profile", table.insert(manager.user.profile, "helloworld"))


    -- 获取最终数据
    print(require("inspect")(manager.raw()))
    -- 输出：
    -- {
    --   user = {
    --     items = { "sword" },
    --     profile = {
    --       age = 25,
    --       name = "Bob"
    --     }
    --   }
    -- }
end)
