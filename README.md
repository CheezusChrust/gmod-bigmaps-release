# Bigger Maps for Garry's Mod

This project aims to increase the maximum map size in Garry's Mod by a factor of two.

For full disclosure, I created this project primarily with the usage of AI tools to disassemble engine files (server.dll, client.dll, engine.dll, vbsp++.exe), review various sources of leaked source engine code, and overall get this project to work. I will not claim to be a reverse engineering nor C++ expert by any means, I was just tired of waiting for somebody else to do this. This project is experimental, use it at your own risk (mainly, of crashing your GMod or your server).

A lot of the tooling is located in a separate research repository on my pc that's a huge mess, I'll do what I can to clean it up and get stuff added here so everything in here can be replicated from scratch.

This module is made specifically for the **latest stable/public** build (at the time of writing this description). It will probably break and need an update as soon as Rubat changes anything.

## Target Platforms
- Windows: 32 and 64 bit client and server
- Linux: 32 bit server

This module will not work on Linux clients (too lazy to set up a linux client), or 64 bit Linux servers (physics doesn't work great on 64 bit linux anyways in general, at least right now).

## Installing

### Server Owners
1. Place the relevant `bigmaps` module for your server in the `<server>/garrysmod/lua/bin/` directory. If bin doesn't exist, create it.
2. Create a file named `bigmaps.lua` in `<server>/garrysmod/lua/autorun/server/` with the following content:

```lua
require("bigmaps")
```

### Regular Players
1. Place the relevant `bigmaps` module for your client in the `<gmod>/garrysmod/lua/bin/` directory. If bin doesn't exist, create it.
2. Navigate to `<gmod>/garrysmod/lua/menu/menu.lua` and add the following line to the *very end of the file, on a new line:*

```lua
require("bigmaps")
```
This second step is necessary to have the module loaded prior to opening a map on clients.

## Development

Whenever I feel like, plus this will break as soon as any update for the main branch of Garry's Mod comes around. Will try to keep it updated but no guarantees.