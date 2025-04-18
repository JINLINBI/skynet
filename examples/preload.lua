-- This file will execute before every lua service start
-- See config

-- print("PRELOAD", ...)


_G.import = require("import").import

_G.log_error = require("skynet").error

