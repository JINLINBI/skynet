-- This file will execute before every lua service start
-- See config

-- print("PRELOAD", ...)


_G.import = require("import").import

_G.log_debug = require("skynet").error
_G.log_info = require("skynet").error
_G.log_warning = require("skynet").error
_G.log_error = require("skynet").error

