@ECHO OFF

REM INPUT ARGUMENTS
set START_TIME=%1
set END_TIME=%2
set PROCESS_NAME=%3

REM FORMAT TIME TAKEN
set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%START_TIME%") do set start_h=%%a&set /a start_m=100%%b %% 100&set /a start_s=100%%c %% 100&set /a start_ms=100%%d %% 100
for /f %options% %%a in ("%END_TIME%") do set end_h=%%a&set /a end_m=100%%b %% 100&set /a end_s=100%%c %% 100&set /a end_ms=100%%d %% 100

set /a hours=%end_h%-%start_h%
set /a mins=%end_m%-%start_m%
set /a secs=%end_s%-%start_s%
set /a ms=%end_ms%-%start_ms%
if %ms% lss 0 set /a secs = %secs% - 1 & set /a ms = 100%ms%
if %secs% lss 0 set /a mins = %mins% - 1 & set /a secs = 60%secs%
if %mins% lss 0 set /a hours = %hours% - 1 & set /a mins = 60%mins%
if %hours% lss 0 set /a hours = 24%hours%
if 1%ms% lss 100 set ms=0%ms%

REM PRINT RUN TIME TO CONSOLE AND BUILD LOG
set /a totalmins=%hours%*60 + %mins%

set TIMER_RESULT=Elapsed Time:   %hours%h:%mins%m:%secs%s.%ms%ms, Total Mins: %totalmins% mins %secs% secs

REM SHORT RESULT IF NO PROCESS NAME
IF %PROCESS_NAME%=="" (
EXIT /B 0 )

REM LONG RESULT WITH PROCESS NAME
ECHO.
ECHO Process:        %PROCESS_NAME%
ECHO Run Date:       %DATE%
ECHO %TIMER_RESULT%
ECHO. 