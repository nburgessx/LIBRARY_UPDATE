@echo off

REM Copy Visual Leak Detector Config File
xcopy /s /y %MLIBQ%\resource\utilities\VisualLeakDetector\vld.ini %MLIBQ%\target\64\DebugVisualLeakDetector\GTEST\out\
xcopy /s /y %MLIBQ%\resource\utilities\VisualLeakDetector\bin\Win64\*.* %MLIBQ%\target\64\DebugVisualLeakDetector\GTEST\out\

@echo on