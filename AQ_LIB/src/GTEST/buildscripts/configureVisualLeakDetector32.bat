@echo off

REM Copy Visual Leak Detector Config File
xcopy /s /y %MLIBQ%\resource\utilities\VisualLeakDetector\vld.ini %MLIBQ%\target\32\DebugVisualLeakDetector\GTEST\out\
xcopy /s /y %MLIBQ%\resource\utilities\VisualLeakDetector\bin\Win32\*.* %MLIBQ%\target\32\DebugVisualLeakDetector\GTEST\out\

@echo on