# Install and use

Use a module built for the supported stock game/server binaries. The module
refuses previously modified or unrecognized files; a new Steam update may need
an updated module. Close the game or server before installing.

## Windows game

1. Put the appropriate module in your game's `garrysmod\lua\bin\` folder:
   `gmsv_bigmaps_win32.dll` for the 32-bit game or
   `gmsv_bigmaps_win64.dll` for the 64-bit game. Install both if you use both.
2. Add this line at the end of `garrysmod\lua\menu\menu.lua`:

   ```lua
   pcall( require, "bigmaps" )
   ```

3. Start the game and load a map. Check the console for `[bigmaps]` messages
   if the module does not load or refuses the game build.

Steam updates or file verification can remove the menu line; add it again if
necessary. Back up any existing menu customizations before editing.

## Dedicated server

1. Put the appropriate module in the server's `garrysmod/lua/bin/`:
   `gmsv_bigmaps_win32.dll`, `gmsv_bigmaps_win64.dll`, or
   `gmsv_bigmaps_linux.dll` for the 32-bit Linux server.
2. Copy [bigmaps.lua](../module/bigmaps/lua/autorun/server/bigmaps.lua) to
   `garrysmod/lua/autorun/server/bigmaps.lua` on the server.
3. Start the server on a **normal-sized map**, then use `changelevel` to load
   an oversized map. Server Lua loads too late to enable an oversized first map.

On an oversized map every client also needs the module. An incompatible client
may be rejected with `Server uses different class tables`. The module restores
normal behavior for normal maps.

## Uninstall

Close the game or server, remove the module file and server loader you installed,
and remove the menu line if you added it. Do not remove unrelated menu changes.
The module does not replace the game engine, client, server, or physics files.
