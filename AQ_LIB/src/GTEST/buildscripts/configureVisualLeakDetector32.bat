@echo off

REM Copy Visual Leak Detector Config File
xcopy /s /y %AQ%\resource\utilities\VisualLeakDetector\vld.ini %AQ%\target\32\DebugVisualLeakDetector\GTEST\out\
xcopy /s /y %AQ%\resource\utilities\VisualLeakDetector\bin\Win32\*.* %AQ%\target\32\DebugVisualLeakDetector\GTEST\out\

@echo on