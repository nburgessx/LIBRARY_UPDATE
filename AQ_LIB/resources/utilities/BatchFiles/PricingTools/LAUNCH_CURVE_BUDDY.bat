@echo off

TITLE LAUNCH CURVE BUDDY

REM In the CurveBuddy GUI, set the Config Folder path to:
REM X:\Secure\DIR_MLib_Analytics\PricingTools\CurveBuddy\BuddyConfig

REM MLIB VERSION CONTROL
call "\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\VersionControl\VersionControl.bat"

REM OVERWRITE THE MLIB ENVIRONMENT VARIABLE
REM WE DO THIS SO THAT QUANTS USE THE SAME CONFIG FOLDER AS END USERS.
set MLIB=""
set MLIBQ=""
set TOOLBAR=%MLIB_TOOLBAR%

REM START "" %EXCEL% /e %TOOLBAR% %CURVE_BUDDY_MLIB_VERSION%

REM Change to the CurveBuddy / MLIBQ folder so that MLIBQ config can be loaded.
cd  "./CurveBuddy/bin"
START "" %CURVE_BUDDY_APP%

cd ../..
EXIT

