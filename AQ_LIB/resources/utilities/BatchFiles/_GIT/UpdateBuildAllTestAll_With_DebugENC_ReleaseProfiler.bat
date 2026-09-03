@echo off

TITLE MLIBQ GIT PULL, BUILD ALL, TEST ALL (INCL. DEBUG ENC AND RELEASE PROFILER)
REM ===============================================

REM RECORD START TIME
set start=%time%

REM TRACK SUCCESS OR FAILURE
SET "STATUS=SUCCESS"


REM CHANGE DIRECTORY TO %MLIBQ%
REM ===============================================

REM CHECK IF MLIBQ ENVIRONMENT VARIABLE HAS BEEN SET
IF %MLIBQ%=="" (
ECHO .
ECHO . *** MLIBQ ENVIRONMENT VARIABLE NOT SET ***
ECHO .
ECHO MLIBQ BUILD NOT RUN
ECHO GOOGLE_TEST RELEASE WIN32 NOT RUN
ECHO GOOGLE_TEST RELEASE X64 NOT RUN
ECHO .
SET "STATUS=*** FAILURE ***"
GOTO :FAILURE )

REM CHANGE DIR TO MLIBQ FOLDER
cd /d %MLIBQ%


REM GIT SYNC WITH REMOTE MASTER
REM ===============================================
set git_start=%time%
git pull -v --progress "origin" master
set git_end=%time%
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\Timer.bat" %git_start%, %git_end%, GitCheckout
set git_time=%TIMER_RESULT%


REM BUILD ALL USING DEBUG ENC (EDIT AND CONTINUE)
REM ===============================================
set build_start=%time%
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\BUILD_SERVER_INCREDIBUILD_ALL_DEBUG_ENC_AND_RELEASE_PROFILER.bat"
set build_end=%time%
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\Timer.bat" %build_start%, %build_end%, VisualStudioBuild
set build_time=%TIMER_RESULT%

REM CHECK IF BUILD WAS SUCCESSFUL
IF %ERRORLEVEL% NEQ 0 (
ECHO .
ECHO MLIBQ BUILD FAILED
ECHO GOOGLE_TEST RELEASE WIN32 NOT RUN
ECHO GOOGLE_TEST RELEASE X64 NOT RUN
ECHO .
SET "STATUS=*** FAILURE ***"
GOTO :FAILURE )


REM TEST ALL
REM ===============================================
set test32_start=%time%
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\RUN_GOOGLE_TEST_WITH_TESTNINJA_CONFIG_2017.bat" 2017 release Win32
set test32_end=%time%
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\Timer.bat" %test32_start%, %test32_end%, RunTests-Release32
set test32_time=%TIMER_RESULT%

REM CHECK IF BUILD IS SUCCESSFUL
IF %ERRORLEVEL% NEQ 0 (
ECHO .
ECHO MLIBQ BUILD SUCCESSFUL
ECHO GOOGLE_TEST RELEASE WIN32 FAILED
ECHO GOOGLE_TEST RELEASE X64 NOT RUN
ECHO .
SET "STATUS=*** FAILURE ***"
GOTO :FAILURE )

set test64_start=%time%
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\RUN_GOOGLE_TEST_WITH_TESTNINJA_CONFIG_2017.bat" 2017 release x64
set test64_end=%time%
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\Timer.bat" %test64_start%, %test64_end%, RunTests-Release64
set test64_time=%TIMER_RESULT%

REM CHECK IF BUILD IS SUCCESSFUL
IF %ERRORLEVEL% NEQ 0 (
ECHO .
ECHO MLIBQ BUILD SUCCESSFUL
ECHO GOOGLE_TEST RELEASE WIN32 SUCCESSFUL
ECHO GOOGLE_TEST RELEASE X64 FAILED
ECHO .
SET "STATUS=*** FAILURE ***"
GOTO :FAILURE )

:SUCCESS
ECHO .
ECHO MLIBQ: %MLIBQ%
ECHO .
ECHO MLIBQ BUILD SUCCESSFUL
ECHO GOOGLE_TEST RELEASE WIN32 SUCCESSFUL
ECHO GOOGLE_TEST RELEASE X64 SUCCESSFUL
ECHO .

:FAILURE
REM RECORD END TIME
set end=%time%

REM TIMER RESULTS
call "%MLIBQ%\resource\utilities\BatchFiles\_GIT\Timer.bat" %start%, %end%, ""
set TOTAL_RESULT=%TIMER_RESULT%

REM PRINT RUN TIME TO CONSOLE
ECHO %STATUS% MLIBQ: %MLIBQ%, Run Date:%DATE%, %TOTAL_RESULT%
ECHO *********************************
ECHO GitCheckout: %git_time%
ECHO VSBuild:     %build_time%
ECHO RunTests32:  %test32_time%
ECHO RunTests64:  %test64_time%
ECHO *********************************
ECHO.

REM UPDATE BUILD LOG
ECHO %STATUS% MLIBQ: %MLIBQ%, Run Date:%DATE%, %TOTAL_RESULT% >> %MLIBQ%\BuildLog-UpdateBuildAllTestAll.txt
ECHO ********************************* >> %MLIBQ%\BuildLog-UpdateBuildAllTestAll.txt
ECHO GitCheckout: %git_time% >> %MLIBQ%\BuildLog-UpdateBuildAllTestAll.txt
ECHO VSBuild:     %build_time% >> %MLIBQ%\BuildLog-UpdateBuildAllTestAll.txt
ECHO RunTests32:  %test32_time% >> %MLIBQ%\BuildLog-UpdateBuildAllTestAll.txt
ECHO RunTests64:  %test64_time% >> %MLIBQ%\BuildLog-UpdateBuildAllTestAll.txt
ECHO ********************************* >> %MLIBQ%\BuildLog-UpdateBuildAllTestAll.txt

rem PAUSE TO READ CONSOLE
PAUSE
