-- Bigger maps (experimental): loads gmsv_bigmaps_win32/_win64/_linux.dll from garrysmod/lua/bin. The module switches
-- itself on for maps beyond +-16384 and off for normal maps at each map change; start the server on a normal map.
require("bigmaps")
