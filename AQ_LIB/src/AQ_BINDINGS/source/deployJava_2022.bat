REM @echo off

REM Copy Java DLL and generated wrapper files
REM Require architecture ( 32 / 64 ) as %1 and configuration as %2

if "%AQ%"=="" ( echo AQ is not set & exit /b 1 )
if "%~1"=="" ( echo Architecture not passed as argument 1 & exit /b 1 )
if "%~2"=="" ( echo Configuration not passed as argument 2 & exit /b 1 )

if not exist "%AQ%\targets" ( echo Targets folder not found: %AQ%\targets & exit /b 1 )
set AQ_BINDINGS_OUT=%AQ%\targets\%1\%2\AQ_BINDINGS\out

if not exist "%AQ_BINDINGS_OUT%\DLL" mkdir "%AQ_BINDINGS_OUT%\DLL"
del /s /f /q "%AQ_BINDINGS_OUT%\DLL" 2>nul

if not exist "%AQ_BINDINGS_OUT%\DLL\config" mkdir "%AQ_BINDINGS_OUT%\DLL\config"
del /s /f /q "%AQ_BINDINGS_OUT%\DLL\config" 2>nul

xcopy /y "%AQ_BINDINGS_OUT%\*.*" "%AQ_BINDINGS_OUT%\DLL"
xcopy /s /y "%AQ%\resources\config\*.*" "%AQ_BINDINGS_OUT%\DLL\config"
xcopy /s /y "%AQ_BINDINGS_OUT%\GeneratedFiles\*.java" "%AQ_BINDINGS_OUT%\DLL"

@echo on