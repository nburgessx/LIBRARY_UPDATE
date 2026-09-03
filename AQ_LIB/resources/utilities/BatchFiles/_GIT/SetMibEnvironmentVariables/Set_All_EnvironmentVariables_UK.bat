@echo off

TITLE Setting All MLIBQ Environment Variable (UK)
setlocal EnableDelayedExpansion
echo.

echo.
echo Setting MLIBQ_LOCATION Environment Variable
echo ===================================================
setx MLIBQ_LOCATION "UK"
ECHO MLIBQ_LOCATION="UK"
ECHO.

echo.
echo Setting MLIBQ Environment Variable
echo =================================
cd %~dp0\..\..\..\..\..
setx MLIBQ "%CD%"
echo %CD%
echo.

echo.
echo Setting MLIB_EXTERNAL_LIB_PATH Environment Variable
echo ===================================================
cd ..
setx MLIB_EXTERNAL_LIB_PATH "%CD%\APPLE_EXTERNAL_LIBS"
echo %CD%\APPLE_EXTERNAL_LIBS
echo.

PAUSE