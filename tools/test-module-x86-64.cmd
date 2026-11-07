@echo off
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" exit /b 1
set "VSROOT="
for /f "usebackq tokens=*" %%I in (`call "%VSWHERE%" -latest -products * -version "[17.0,18.0)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT exit /b 1
call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.19041.0 >nul
if errorlevel 1 exit /b 1
set "DIR=%~dp0..\work\module-x86-64\tests"
if not exist "%DIR%" mkdir "%DIR%"
if errorlevel 1 exit /b 1
pushd "%DIR%"
if errorlevel 1 exit /b 1
cl /nologo /Zs /MT /O2 /EHsc /W4 /WX /DBM_BRANCH_SYNTAX_CHECK "%~dp0..\module-x86-64\bigmaps\bigmaps.cpp"
if errorlevel 1 (popd & exit /b 1)
cl /nologo /MT /O2 /EHsc /W4 /WX "%~dp0..\module-x86-64\tests\sound_trampoline.cpp" /Fe:sound_trampoline_test.exe
if errorlevel 1 (popd & exit /b 1)
sound_trampoline_test.exe
if errorlevel 1 (popd & exit /b 1)
cl /nologo /MT /O2 /EHsc /W4 /WX "%~dp0..\module-x86-64\tests\sound_hook_state.cpp" /Fe:sound_hook_state_test.exe
if errorlevel 1 (popd & exit /b 1)
sound_hook_state_test.exe
if errorlevel 1 (popd & exit /b 1)
cl /nologo /MT /O2 /EHsc /W4 /WX "%~dp0..\module-x86-64\tests\patch_site_state.cpp" /Fe:patch_site_state_test.exe
if errorlevel 1 (popd & exit /b 1)
patch_site_state_test.exe
if errorlevel 1 (popd & exit /b 1)
popd
exit /b 0