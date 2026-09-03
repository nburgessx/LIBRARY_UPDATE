@echo off

TITLE MLIBQ Linux Build

REM	BUILD RELEASE / Linux 64bit
"C:\Program Files\Putty\plink.exe" -pw e4jPLezoetDi svc_quant_dev@adl-quant01 /home2/svc_quant_dev/scripts/svc_quant_dev_git_build_and_test.sh

rem Pause to Read Console
rem ---------------------
if "%4%"=="PAUSE" PAUSE