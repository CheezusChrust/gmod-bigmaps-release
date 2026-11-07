@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: Microsoft vswhere.exe was not found. 1>&2
    exit /b 1
)
set "VSROOT="
for /f "usebackq tokens=*" %%I in (`call "%VSWHERE%" -latest -products * -version "[17.0,18.0)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT (
    echo ERROR: Visual Studio 2022 with x86/x64 C++ tools was not found. 1>&2
    exit /b 1
)
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.19041.0 >nul
if errorlevel 1 exit /b 1
set "DIR=%~dp0..\work\module-x86-64\bigmaps\x64"
if not exist "%DIR%" mkdir "%DIR%"
if errorlevel 1 exit /b 1
pushd "%DIR%"
if errorlevel 1 exit /b 1
cl /nologo /LD /MT /O2 /EHsc /W4 /WX "%~dp0..\module-x86-64\bigmaps\bigmaps.cpp" /Fe:gmsv_bigmaps_win64.dll /link /MACHINE:X64
if errorlevel 1 (popd & exit /b 1)
dumpbin /nologo /headers /exports gmsv_bigmaps_win64.dll > inspection.txt
if errorlevel 1 (popd & exit /b 1)
popd
echo Module build OK: %DIR%
exit /b 0