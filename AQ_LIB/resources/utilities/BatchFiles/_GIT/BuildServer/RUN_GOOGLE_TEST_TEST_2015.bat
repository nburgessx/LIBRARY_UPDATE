@echo off

REM 	Arg1 = debug/release
REM	Arg2 = 32/64
REM	Arg3 = google test command e.g. --gtest_repeat=3
REM	Arg4 = google test command e.g. --pause

CALL RUN_GOOGLE_TEST_CONFIG_2015.bat release 32 "--gtest_repeat=1" --pause
