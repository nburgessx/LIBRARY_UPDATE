@echo off

REM BUILD SERVER TEST SCRIPT

REM 	Arg1 = build/rebuild/clean
REM	Arg2 = debug/release
REM	Arg3 = win32/x64
REM     Arg4 = PAUSE or leave blank
CALL BUILD_SERVER_INCREDIBUILD_CONFIG.bat build release win32 PAUSE 