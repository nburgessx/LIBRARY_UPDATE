@echo off


TITLE MLIBQ Linux Build

REM	BUILD RELEASE / Linux 64bit
D:
cd %MLIBQ%

if exist src.zip del src.zip
if exist resource.zip del resource.zip

c:\Rtools\bin\zip -r src.zip src
c:\Rtools\bin\zip -r resource.zip resource/test resource/config resource/utilities/Apps/TestNinjaPython

"C:\Program Files\Putty\pscp.exe" -pw bcFj2bTx src.zip resource.zip  qnt_user@10.153.2.249:scripts/.
"C:\Program Files\Putty\plink.exe" -pw bcFj2bTx qnt_user@10.153.2.249 /home/qnt_user/scripts/qnt_user_git_build_and_test.sh

rem Pause to Read Console
rem ---------------------
if "%4%"=="PAUSE" PAUSE