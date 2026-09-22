@echo off
REM ============================================================================
REM  SetEnvVars.bat
REM
REM  Sets the user environment variables required to build AlgoQuantLib.
REM  Run once from this folder, then RESTART Visual Studio (MSBuild reads the
REM  environment at launch, so a running VS will not see these).
REM
REM  AQ                    Root of the AQ_LIB source tree. Derived from this
REM                        script's location - not prompted for.
REM  AQ_EXTERNAL_LIB_PATH  Root of AQ_EXTERNAL_LIBS (Boost, QuantLib, Adept,
REM                        Eigen, rapidjson, SWIG).
REM  AQ_PYTHON_ROOT        Python install the bindings are COMPILED AGAINST.
REM                        Only needed for the ReleasePython configuration.
REM                        Must be 64-bit with Include\Python.h and libs\.
REM                        A .pyd is locked to the minor version it was built
REM                        against, so this must match the Python you run it
REM                        under. The vendored copies under AQ_EXTERNAL_LIBS
REM                        are legacy (2.7/3.6/3.7) and will not load on a
REM                        modern machine.
REM
REM  Note: the config folder (calendars + generator JSON, CLAUDE.md Sec 4.3)
REM  ships next to the AlgoQuantLib module itself and is found automatically
REM  at that fixed location - it has no environment variable here.
REM
REM  Prompts default to your existing value if set, otherwise to a detected
REM  one. Press Enter to accept, or type a path to override. Skipping Python
REM  leaves any existing AQ_PYTHON_ROOT untouched.
REM
REM  Edit the DEFAULT_* values below to change what is suggested on a machine
REM  with nothing set yet.
REM ============================================================================

setlocal EnableDelayedExpansion

REM Resolve this script's folder, dropping the trailing backslash
set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

REM Capture what is already set, before we overwrite anything locally
set "EXISTING_AQ=%AQ%"
set "EXISTING_EXT=%AQ_EXTERNAL_LIB_PATH%"
set "EXISTING_PY=%AQ_PYTHON_ROOT%"

REM ---------------------------------------------------------------------------
REM  DEFAULTS - used when nothing is set yet
REM ---------------------------------------------------------------------------

set DEFAULT_AQ=%SCRIPT_DIR%\..\AQ_LIB
set DEFAULT_AQ_EXTERNAL_LIB_PATH=%SCRIPT_DIR%\..\AQ_EXTERNAL_LIBS

REM Python is detected from PATH. To suggest a fixed install instead, replace
REM the loop below with e.g.  set DETECTED_PY=C:\Users\me\anaconda3
set DETECTED_PY=

for /f "delims=" %%P in ('where python 2^>nul') do (
    if "!DETECTED_PY!"=="" (
        REM Skip the Microsoft Store stub - not a real install
        echo %%P | find /i "WindowsApps" >nul
        if errorlevel 1 set DETECTED_PY=%%~dpP
    )
)
if not "%DETECTED_PY%"=="" set DETECTED_PY=%DETECTED_PY:~0,-1%

REM Expand ..\ into absolute paths
for %%I in ("%DEFAULT_AQ%") do set DEFAULT_AQ=%%~fI
for %%I in ("%DEFAULT_AQ_EXTERNAL_LIB_PATH%") do set DEFAULT_AQ_EXTERNAL_LIB_PATH=%%~fI

REM Existing value wins over the detected/derived default
set "SUGGEST_EXT=%DEFAULT_AQ_EXTERNAL_LIB_PATH%"
if not "%EXISTING_EXT%"=="" set "SUGGEST_EXT=%EXISTING_EXT%"

set "SUGGEST_PY=%DETECTED_PY%"
if not "%EXISTING_PY%"=="" set "SUGGEST_PY=%EXISTING_PY%"

REM ---------------------------------------------------------------------------
REM  Show what is currently set
REM ---------------------------------------------------------------------------

echo.
echo  AlgoQuantLib environment setup
echo  ==============================
echo.
echo  Currently set:
if "%EXISTING_AQ%"=="" (
    echo    AQ                   ^(not set^)
) else (
    echo    AQ                   %EXISTING_AQ%
)
if "%EXISTING_EXT%"=="" (
    echo    AQ_EXTERNAL_LIB_PATH ^(not set^)
) else (
    echo    AQ_EXTERNAL_LIB_PATH %EXISTING_EXT%
)
if "%EXISTING_PY%"=="" (
    echo    AQ_PYTHON_ROOT       ^(not set^)
) else (
    echo    AQ_PYTHON_ROOT       %EXISTING_PY%
)
echo.
echo  Press Enter at each prompt to accept the suggested value.
echo.

REM ---------------------------------------------------------------------------
REM  AQ - derived from this script's location, no prompt
REM ---------------------------------------------------------------------------

set AQ=%DEFAULT_AQ%
if not exist "%AQ%\src" (
    echo  [ERROR] No src folder under the derived AQ path:
    echo          %AQ%
    echo          This script must sit next to the solution, one level below
    echo          the folder containing AQ_LIB. Nothing written.
    echo.
    pause
    endlocal
    exit /b 1
)
echo  AQ  =  %AQ%
echo.

REM ---------------------------------------------------------------------------
REM  AQ_EXTERNAL_LIB_PATH
REM ---------------------------------------------------------------------------

:PROMPT_EXT
echo  External libraries (AQ_EXTERNAL_LIB_PATH)
echo    suggested: %SUGGEST_EXT%
set "AQ_EXTERNAL_LIB_PATH="
set /p "AQ_EXTERNAL_LIB_PATH=  Path [Enter=suggested]: "
if "%AQ_EXTERNAL_LIB_PATH%"=="" set AQ_EXTERNAL_LIB_PATH=%SUGGEST_EXT%
for %%I in ("%AQ_EXTERNAL_LIB_PATH%") do set AQ_EXTERNAL_LIB_PATH=%%~fI

if not exist "%AQ_EXTERNAL_LIB_PATH%\library" (
    echo    [!] No library folder found under that path. Try again.
    echo.
    goto PROMPT_EXT
)
echo.

REM ---------------------------------------------------------------------------
REM  Python - opt in or out first, then ask for the path
REM ---------------------------------------------------------------------------

set SETUP_PYTHON=

:PROMPT_PY_YN
set "PY_ANSWER="
set /p "PY_ANSWER=  Set up Python environment? Y/N [Enter=N]: "
if "%PY_ANSWER%"=="" set PY_ANSWER=N
if /i "%PY_ANSWER%"=="Y" set SETUP_PYTHON=1
if /i "%PY_ANSWER%"=="N" set SETUP_PYTHON=0
if "%SETUP_PYTHON%"=="" (
    echo    Invalid response. Enter Y or N.
    goto PROMPT_PY_YN
)
echo.

if "%SETUP_PYTHON%"=="0" goto SUMMARY

:PROMPT_PY_PATH
echo  Python install (AQ_PYTHON_ROOT)
if "%SUGGEST_PY%"=="" (
    echo    no Python detected on PATH - enter a path
) else (
    echo    suggested: %SUGGEST_PY%
)
set "AQ_PYTHON_ROOT="
set /p "AQ_PYTHON_ROOT=  Path [Enter=suggested]: "
if "%AQ_PYTHON_ROOT%"=="" set AQ_PYTHON_ROOT=%SUGGEST_PY%

if "%AQ_PYTHON_ROOT%"=="" (
    echo    [!] No path given and none detected. Try again.
    echo.
    goto PROMPT_PY_PATH
)
for %%I in ("%AQ_PYTHON_ROOT%") do set AQ_PYTHON_ROOT=%%~fI

if not exist "%AQ_PYTHON_ROOT%\Include\Python.h" (
    echo    [!] Python.h not found under %AQ_PYTHON_ROOT%\Include
    echo.
    goto PROMPT_PY_PATH
)
if not exist "%AQ_PYTHON_ROOT%\libs" (
    echo    [!] libs folder not found under %AQ_PYTHON_ROOT%
    echo.
    goto PROMPT_PY_PATH
)

REM Query version and bitness - a 32-bit Python cannot link into an x64 build,
REM and that failure otherwise shows up as an obscure linker error much later
set "PY_VERSION="
set "PY_BITS="
if exist "%AQ_PYTHON_ROOT%\python.exe" (
    for /f "delims=" %%V in ('"%AQ_PYTHON_ROOT%\python.exe" -c "import sys;print(sys.version.split()[0])" 2^>nul') do set PY_VERSION=%%V
    for /f "delims=" %%B in ('"%AQ_PYTHON_ROOT%\python.exe" -c "import struct;print(struct.calcsize('P')*8)" 2^>nul') do set PY_BITS=%%B
)
if "%PY_BITS%"=="32" (
    echo    [!] This is a 32-bit Python. The x64 build cannot link against it.
    echo        Install 64-bit Python, or enter a different path.
    echo.
    goto PROMPT_PY_PATH
)
echo.

REM ---------------------------------------------------------------------------
REM  Summary
REM ---------------------------------------------------------------------------

:SUMMARY
echo  ------------------------------------------------------------
echo   AQ                    = %AQ%
echo   AQ_EXTERNAL_LIB_PATH  = %AQ_EXTERNAL_LIB_PATH%
if "%SETUP_PYTHON%"=="0" (
    if "%EXISTING_PY%"=="" (
        echo   AQ_PYTHON_ROOT        = ^(skipped, not set^)
    ) else (
        echo   AQ_PYTHON_ROOT        = %EXISTING_PY%  ^(skipped, unchanged^)
    )
) else (
    echo   AQ_PYTHON_ROOT        = %AQ_PYTHON_ROOT%
    if not "%PY_VERSION%"=="" echo   Python target         = %PY_VERSION%  %PY_BITS%-bit
)
echo  ------------------------------------------------------------
if "%SETUP_PYTHON%"=="0" (
    if "%EXISTING_PY%"=="" (
        echo.
        echo  The ReleasePython configuration will not build until
        echo  AQ_PYTHON_ROOT is set. Re-run this script any time to add it.
    )
)
echo.

choice /c YN /m "Write these to your user environment"
if errorlevel 2 (
    echo Cancelled. Nothing written.
    echo.
    pause
    endlocal
    exit /b 0
)

setx AQ "%AQ%" >nul
setx AQ_EXTERNAL_LIB_PATH "%AQ_EXTERNAL_LIB_PATH%" >nul
if "%SETUP_PYTHON%"=="1" setx AQ_PYTHON_ROOT "%AQ_PYTHON_ROOT%" >nul

echo.
echo  Done. RESTART VISUAL STUDIO - setx does not update running processes.
echo.
pause

endlocal
exit /b 0