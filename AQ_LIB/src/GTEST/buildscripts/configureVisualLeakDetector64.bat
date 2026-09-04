@echo off

REM Copy Visual Leak Detector Config File
xcopy /s /y %AQ%\resource\utilities\VisualLeakDetector\vld.ini %AQ%\target\64\DebugVisualLeakDetector\GTEST\out\
xcopy /s /y %AQ%\resource\utilities\VisualLeakDetector\bin\Win64\*.* %AQ%\target\64\DebugVisualLeakDetector\GTEST\out\

@echo on