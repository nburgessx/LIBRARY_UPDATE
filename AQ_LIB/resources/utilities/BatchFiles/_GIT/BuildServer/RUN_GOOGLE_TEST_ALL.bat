@echo off

TITLE Running Google Test: DEBUG WIN32
set GoogleTest="%MLIBQ%\target2015\32\Debug\google_test\out\google_test.exe"
%GoogleTest%

TITLE Running Google Test: RELEASE WIN32
set GoogleTest="%MLIBQ%\target2015\32\Release\google_test\out\google_test.exe"
%GoogleTest%

TITLE Running Google Test: DEBUG x64
set GoogleTest="%MLIBQ%\target2015\64\Debug\google_test\out\google_test.exe"
%GoogleTest%

TITLE Running Google Test: RELEASE x64
set GoogleTest="%MLIBQ%\target2015\64\Release\google_test\out\google_test.exe"
%GoogleTest%