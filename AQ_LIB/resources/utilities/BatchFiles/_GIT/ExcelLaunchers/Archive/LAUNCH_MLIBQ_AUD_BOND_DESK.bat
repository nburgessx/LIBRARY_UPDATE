@echo off

REM MLIB VERSION CONTROL
call "\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\VersionControl\VersionControl.bat"

REM OVERWRITE THE MLIB ENVIRONMENT VARIABLE
set MLIB=""
set TOOLBAR=%MLIB_TOOLBAR%

START "" %EXCEL% /e /r %MLIB_AUD_BOND_DESK% %TOOLBAR%
EXIT