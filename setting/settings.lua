local skynet = require "skynet"


if skynet.getenv("isdebug") then
    return require "settings_template_debug"
else
    return require "settings_template"
end