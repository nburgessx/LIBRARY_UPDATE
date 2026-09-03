@echo off

REM Force rebuild of *.cxx by blowing away the exposed_functions.obj file
REM Require architecture ( 32 / 64 ) as %1 and configuration as %2

if "%AQ%"=="" ( echo AQ is not set & exit /b 1 )
if "%AQ_EXTERNAL_LIB_PATH%"=="" ( echo AQ_EXTERNAL_LIB_PATH is not set & exit /b 1 )
if "%~1"=="" ( echo Architecture not passed as argument 1 & exit /b 1 )
if "%~2"=="" ( echo Configuration not passed as argument 2 & exit /b 1 )

if not exist "%AQ%\targets" ( echo Targets folder not found: %AQ%\targets & exit /b 1 )
set AQ_API_TARGET=%AQ%\targets\%1\%2\AQ_API
del /s /f /q "%AQ_API_TARGET%\obj\exposed_functions.obj" 2>nul

REM Make the output folder if this is the first time it is being built
set GENERATED_FILES=%AQ_API_TARGET%\out\GeneratedFiles
if not exist "%GENERATED_FILES%" mkdir "%GENERATED_FILES%"

REM Clear Previous Generated Files
del /s /f /q "%GENERATED_FILES%\*.*" 2>nul

REM This places the Client *.cs files into the GeneratedFiles output folder,
REM and places swig_CSharp_wrap.cxx in the src\AQ_API\source folder
"%AQ_EXTERNAL_LIB_PATH%\library\swigwin-4.0.0\swig.exe" -v -c++ -outdir "%GENERATED_FILES%" -o "%AQ%\src\AQ_API\source\swig_CSharp_wrap.cxx" -csharp "%AQ%\src\AQ_API\source\swig_CSharp.i"

REM After running this, add the pre-compiled header include statement to the generate XXXXX_wrap.cxx and rebuild the project if using pre-compiled headers
@echo on