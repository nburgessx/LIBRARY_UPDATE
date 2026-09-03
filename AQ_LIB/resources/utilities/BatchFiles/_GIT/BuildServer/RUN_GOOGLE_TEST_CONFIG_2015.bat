@echo off

REM 	Arg1 = debug/release
REM	Arg2 = 32/64
REM	Arg3 = google test command e.g. --gtest_repeat=3
REM	Arg4 = google test command e.g. --pause
TITLE Running Google Test: %1 %2 with Google Command : %3 %4

set GoogleTest="%MLIBQ%\target2015\%2\%1\google_test\out\google_test.exe"

%GoogleTest% %3 %4
