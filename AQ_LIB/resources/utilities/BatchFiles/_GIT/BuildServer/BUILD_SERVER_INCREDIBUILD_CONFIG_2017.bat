@echo off

REM 	Arg1 = build/rebuild/clean
REM	Arg2 = debug/release
REM	Arg3 = win32/x64
REM     Arg4 = PAUSE or leave blank

TITLE MLIBQ %1 : Incredibuild %2 %3

REM Read Folder Control variables
IF NOT DEFINED INCREDIBUILD (SET INCREDIBUILD="C:\Program Files (x86)\Xoreax\IncrediBuild")

REM Check Incredibuild Installed
IF not exist "%INCREDIBUILD%" (
ECHO .
ECHO INCREDIBUILD NOT FOUND
ECHO Please ensure IncrediBuild is installed in "C:\Program Files (x86)\Xoreax\IncrediBuild"
ECHO or set the INCREDIBUILD Environment Variable if IncredibBuild has been custom installed elsewhere.
ECHO .
PAUSE
EXIT )
	
REM BUILD DEBUG / RELEASE WIN32
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /VsVersion=vc14.1 /%1 /cfg="%2|%3"

rem Pause to Read Console
rem ---------------------
if "%4%"=="PAUSE" PAUSE