@echo off

REM	Arg1 = 2010 / 2015
REM 	Arg2 = debug/release
REM	Arg3 = Win32 / x64
REM	Arg4 = google test filter
TITLE Running Google Test: Visual Studio %1 Build %2 Architecture %3

set TestNinjaCommand="%MLIBQ%\resource\utilities\BatchFiles\TestNinja\TestNinja.exe"

SET MYFILTER="%~4"
IF %MYFILTER%=="" (SET MYFILTER=*.*)

%TestNinjaCommand% --visualStudio %1 --build %2 --arch %3 --threads 8 --workspace %MLIBQ% --filter %MYFILTER%