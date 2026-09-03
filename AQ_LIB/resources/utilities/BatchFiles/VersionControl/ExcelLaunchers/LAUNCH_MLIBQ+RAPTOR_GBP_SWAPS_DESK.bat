@echo off

REM EXCEL SWITCHES
REM -------------------------------
REM /e = embed Excel and don't show splash screen
REM /x = start a new instance of Excel
REM /r = open files to switch RHS as read only

REM MLIB VERSION CONTROL
call "\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\VersionControl\VersionControl.bat"

REM OVERWRITE THE MLIB ENVIRONMENT VARIABLE
set MLIB=""
set TOOLBAR=%MLIB_TOOLBAR%

REM RAPTOR VERSION
call "\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\VersionControl\RaptorVersion.bat"
SET RAPTOR_ADDIN=%RAPTOR_X64%
IF %IS_64_BIT%==FALSE (SET RAPTOR_ADDIN=%RAPTOR_X86%)

REM CHECK IF USER HAS RAPTOR ACCESS RIGHTS
if EXIST %RAPTOR_ADDIN% (
	GOTO :RAPTOR_USER
)

:WARNING_USER_WITHOUT_RAPTOR_ACCESS
echo .
echo *** WARNING - USER HAS NO ACCESS RIGHTS TO RAPTOR XL ***
echo You do not have Access Rights to Raptor XL.
echo .
echo RAPTOR PATH: %RAPTOR_ADDIN%
echo .
echo ACCESS RIGHTS: Please contact the Raptor Development Team, email: "DL-MHI-RAPTORDEVELOPERS" or the IT Service Desk to request access.
echo .
echo YOUR OPTIONS: Close this command line pop-up or press any key to continue to Launch Excel and the MLIBQ Quant Analytics without Raptor access
echo .
PAUSE

REM OPEN XL WITH MLIB ONLY
START "" %EXCEL% /e /x /r %MLIB_GBP_SWAPS_DESK% %MLIB_TOOLBAR%
EXIT

:RAPTOR_USER
REM OPEN XL WITH MLIB + RAPTOR
START "" %EXCEL% /e /x /r %MLIB_GBP_SWAPS_DESK% %RAPTOR_ADDIN% %MLIB_TOOLBAR%
EXIT
