@echo off

REM MLIB VERSION CONTROL
call "\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\VersionControl\VersionControl.bat"

REM OVERWRITE THE MLIB ENVIRONMENT VARIABLE
set MLIB=""
set TOOLBAR=%MLIB_TOOLBAR%

REM /e = embed Excel and don't show splash screen
REM /x = start a new instance of Excel
REM /r = open files to switch RHS as read only
START "" %EXCEL% /e /x /r %MLIB_CURRENT% %MLIB_TOOLBAR%
EXIT