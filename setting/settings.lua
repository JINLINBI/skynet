local skynet = require "skynet"


if skynet.getenv("isdebug") == true then
    log_debug("USING DEBUG CONFIGS!!!! settings.settings_template_debug.lua")
    return require "settings_template_debug"
else
    return require "settings_template"
end