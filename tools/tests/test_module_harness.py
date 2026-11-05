"""Exercise the disposable-server harness without starting a server."""
import contextlib
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import test_module as harness


class ModuleHarness(unittest.TestCase):
    def test_requires_built_module(self):
        with tempfile.TemporaryDirectory() as d:
            with mock.patch.object(harness, "REPO", Path(d)), \
                    mock.patch.object(sys, "argv", ["test_module.py", "x64"]), \
                    mock.patch.object(harness.subprocess, "Popen") as start:
                with self.assertRaisesRegex(SystemExit, "Build the module first"):
                    harness.main()
                start.assert_not_called()

    def test_starts_runtime_module_without_probe_or_commands(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            server = root / "work" / "gmod-ds"
            server.mkdir(parents=True)
            (server / "srcds_win64.exe").write_bytes(b"test executable")
            dll = root / "work" / "module" / "bigmaps" / "x64" / "gmsv_bigmaps_win64.dll"
            dll.parent.mkdir(parents=True)
            dll.write_bytes(b"test module")
            loader = root / "loader.lua"
            loader.write_text('require("bigmaps")', encoding="ascii")
            proc = mock.Mock()
            proc.poll.return_value = None

            def start_server(args, **kwargs):
                (server / "garrysmod" / "console.log").write_text(harness.DONE, encoding="ascii")
                return proc

            with mock.patch.object(harness, "REPO", root), \
                    mock.patch.object(harness, "LOADER", loader), \
                    mock.patch.object(sys, "argv", ["test_module.py", "x64"]), \
                    mock.patch.object(harness, "port_busy", return_value=False), \
                    mock.patch.object(harness.time, "sleep"), \
                    mock.patch.object(harness.subprocess, "Popen", side_effect=start_server) as start, \
                    contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(harness.main(), 0)
                args = start.call_args.args[0]
                self.assertEqual(args[0], str(server / "srcds_win64.exe"))
                self.assertEqual(args[-2:], ["+map", "gm_construct"])
                self.assertEqual(
                    (server / "garrysmod" / "lua" / "bin" / dll.name).read_bytes(),
                    dll.read_bytes(),
                )
                self.assertTrue((server / "garrysmod" / "lua" / "autorun" / "server" / "bigmaps.lua").is_file())
                proc.terminate.assert_not_called()
                proc.kill.assert_not_called()
                proc.communicate.assert_not_called()
                self.assertFalse(any("probe" in str(p) for p in root.rglob("*")))


if __name__ == "__main__":
    unittest.main(verbosity=2)
