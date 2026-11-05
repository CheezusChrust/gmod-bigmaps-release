"""Load a bigger-maps module on a work dedicated server and check its console output.

Usage (repository root):
  python tools\\test_module.py x86|x64 [--map gm_construct] [--timeout 120]
  python tools\\test_module.py x86|x64 --log     (console text since the last start; cleans up the
                                                 test files if the server has been closed)

Installs the module DLL into the work server's garrysmod\\lua\\bin plus autorun files, starts srcds with
-condebug in its own console window and follows the new part of garrysmod\\console.log until the
"BIGMAPS TEST DONE" banner (or srcds exits, or a fatal engine message), then prints the module lines.
The real loader requires the module, which switches itself per map at map changes;
a test autorun prints the map and the banner on every map load.
The files stay installed while the server runs (the module and loader must also run on later maps)
and are removed by the next run or by --log once the server is closed.
It sends no commands and never stops the server: the owner runs changelevel/connect/quit.
Never touches the Steam install.
"""
import argparse
import pathlib
import subprocess
import sys
import time

REPO = pathlib.Path(__file__).resolve().parent.parent
ARCH = {
    "x64": ("work/gmod-ds", "srcds_win64.exe", "win64"),
    "x86": ("work/gmod-ds-x86", "srcds.exe", "win32"),
}
DONE = "BIGMAPS TEST DONE"
BANNER = """print("==================================================================")
print("{done} - module loaded; continue in this window or close it (quit)")
print("==================================================================")
"""
STATUS_AUTORUN = """print("[bigmaps_test] map " .. game.GetMap())
""" + BANNER
LOADER = REPO / "module/bigmaps/lua/autorun/server/bigmaps.lua"
# Files the harness may install (relative to the server's garrysmod folder).
TEST_FILES = ["lua/autorun/server/zz_bigmaps_test.lua",
              "lua/autorun/server/bigmaps.lua"]


def port_busy(port=27025):
    out = subprocess.run(["netstat", "-ano", "-p", "udp"], capture_output=True, text=True).stdout
    return any(f":{port} " in line for line in out.splitlines())


def cleanup(root, suffix):
    removed = []
    names = TEST_FILES + [f"lua/bin/gmsv_bigmaps_{suffix}.dll"]
    for rel in names:
        p = root / "garrysmod" / rel
        try:
            if p.exists():
                p.unlink()
                removed.append(rel)
        except PermissionError:
            print("Note: could not remove (in use):", p)
    if removed:
        print("Removed test files:", ", ".join(removed))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("arch", choices=ARCH)
    ap.add_argument("--map", default="gm_construct")
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--log", action="store_true")
    a = ap.parse_args()

    root_rel, exe, suffix = ARCH[a.arch]
    root = REPO / root_rel
    log = root / "garrysmod/console.log"
    mark = REPO / "work/module" / f"last-start-offset-{a.arch}.txt"
    if a.log:
        start = int(mark.read_text()) if mark.exists() else 0
        with open(log, "rb") as f:
            f.seek(start)
            sys.stdout.write(f.read().decode("utf-8", "replace"))
        if not port_busy():
            cleanup(root, suffix)
        return 0

    mod = "bigmaps"
    name = f"gmsv_{mod}_{suffix}.dll"
    dll = REPO / "work/module/bigmaps" / a.arch / name
    if not dll.is_file():
        sys.exit("Build the module first: tools\\build-module.cmd")
    if not (root / exe).is_file():
        sys.exit(f"Server not installed: {root}")
    if port_busy():
        sys.exit("Port 27025 is already in use. Close the other server first.")
    cleanup(root, suffix)

    start = log.stat().st_size if log.exists() else 0
    mark.parent.mkdir(parents=True, exist_ok=True)
    mark.write_text(str(start))

    def new_text():
        if not log.exists():
            return ""
        with open(log, "rb") as f:
            f.seek(start)
            return f.read().decode("utf-8", "replace")

    autorun = root / "garrysmod/lua/autorun/server"
    (root / "garrysmod/lua/bin").mkdir(parents=True, exist_ok=True)
    autorun.mkdir(parents=True, exist_ok=True)
    (root / "garrysmod/lua/bin" / name).write_bytes(dll.read_bytes())
    (autorun / "bigmaps.lua").write_bytes(LOADER.read_bytes())
    (autorun / "zz_bigmaps_test.lua").write_text(STATUS_AUTORUN.format(done=DONE), encoding="ascii")

    args = [str(root / exe), "-console", "-game", "garrysmod", "-insecure", "-port", "27025",
            "-condebug", "+sv_lan", "1", "+maxplayers", "2", "+map", a.map]
    print(f"Starting {a.arch} server ({exe}, map {a.map}) with module {mod}...", flush=True)
    # Own console window, no handle redirection: srcds exits with an Engine error on a NUL stdin.
    proc = subprocess.Popen(args, cwd=root, creationflags=subprocess.CREATE_NEW_CONSOLE)

    # Stop waiting (never the server) on the banner, a fatal engine message, or the process exiting.
    fatal = ("Host_EndGame", "Host_Error", "Engine error", "Map coordinate extents")
    deadline = time.time() + a.timeout
    text, why = "", f"no '{DONE}' line within {a.timeout}s"
    while time.time() < deadline:
        time.sleep(2)
        text = new_text()
        if DONE in text:
            why = ""
            break
        hit = next((f for f in fatal if f in text), None)
        if hit:
            why = f"server logged '{hit}' before the banner"
            break
        if proc.poll() is not None:
            why = f"server exited (code {proc.returncode}) before the banner"
            break
    time.sleep(1)
    text = new_text()

    out = REPO / "work/module" / f"last-bigmaps-{a.arch}-console.log"
    out.write_text(text, encoding="utf-8")
    print("--- relevant new console lines ---")
    keys = ("[bigmaps", "ERROR", "odule", "rror", "extents", "Host_", "different class")
    for line in text.splitlines():
        if any(k in line for k in keys):
            print(line)
    print("Full new console text:", out)
    if why:
        print(f"Stopped waiting: {why}.")
    state = "has exited" if proc.poll() is not None else "is still running for the owner"
    print(f"Finished ({a.arch}, {mod}). The server {state}.")
    return 0 if DONE in text else 1


if __name__ == "__main__":
    sys.exit(main())
