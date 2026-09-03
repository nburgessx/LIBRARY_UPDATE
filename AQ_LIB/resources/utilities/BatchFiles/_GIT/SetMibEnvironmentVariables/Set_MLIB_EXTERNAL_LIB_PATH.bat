@echo off

TITLE Setting MLIBQ Environment Variable(s)
setlocal EnableDelayedExpansion
echo.

echo.
echo Setting MLIBQ Environment Variable
echo =================================
cd %~dp0\..\..\..\..\..
setx MLIBQ "%CD%%"
echo %CD%
echo.

echo.
echo Setting MLIB_EXTERNAL_LIB_PATH Environment Variable
echo ===================================================
cd ..
setx MLIB_EXTERNAL_LIB_PATH "%CD%"\APPLE_EXTERNAL_LIBS
echo %CD%\APPLE_EXTERNAL_LIBS
echo.

PAUSE