@echo off

TITLE Setting MLIBQ Environment Variable(s)
setlocal EnableDelayedExpansion
echo.

echo Setting MLIBQ Environment Variable
echo =================================
REM Current Directory will be the Batch File Launch Directory
cd %~dp0\..\..\..\..\..
set "FIVE_FOLDERS_UP=%cd%"
setx MLIBQ "%FIVE_FOLDERS_UP%"
echo %FIVE_FOLDERS_UP%
echo.

PAUSE