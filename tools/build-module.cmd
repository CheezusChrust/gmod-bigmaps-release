@echo off
rem Builds the module for both Windows architectures:
rem   work\module\bigmaps\x86|x64\gmsv_bigmaps_win32|win64.dll
rem Usage: tools\build-module.cmd
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
set "MOD=%~dp0..\module"
set "OUT=%~dp0..\work\module"
call :build x86 win32 X86 bigmaps bigmaps "%MOD%\bigmaps\bigmaps.cpp" || exit /b 1
call :build x64 win64 X64 bigmaps bigmaps "%MOD%\bigmaps\bigmaps.cpp" || exit /b 1
echo Module build OK: %OUT%
exit /b 0

rem :build ARCH SUFFIX MACHINE FOLDER NAME SOURCE
:build
setlocal
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" %1 10.0.19041.0 >nul
if errorlevel 1 exit /b 1
set "DIR=%OUT%\%4\%1"
if not exist "%DIR%" mkdir "%DIR%"
if errorlevel 1 exit /b 1
pushd "%DIR%"
if errorlevel 1 exit /b 1
cl /nologo /LD /MT /O2 /EHsc /W4 /WX %6 /Fe:gmsv_%5_%2.dll /link /MACHINE:%3
if errorlevel 1 (popd & exit /b 1)
dumpbin /nologo /headers /exports gmsv_%5_%2.dll > inspection.txt
if errorlevel 1 (popd & exit /b 1)
popd
endlocal
exit /b 0
