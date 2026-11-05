# Agent and contributor notes

- Read [README.md](README.md), [development](docs/DEVELOPMENT.md), and
  [testing](docs/TESTING.md) before changing behavior.
- Preserve stock-file hash and memory checks; never silently accept an unknown
  game build or overwrite a user's game installation. Work on copies.
- Do not copy code from leaked sources or commit game binaries, build outputs,
  test installations, credentials or other secrets.
- Runtime and compiler changes are independent. Verify each affected platform
  and document actual tests and limitations; do not claim untested compatibility.
- Use the [development tools](tools/README.md) for read-only porting and offline
  checks. Detailed historical evidence remains in the separate research project.
  These tools are not required for a normal module build.
