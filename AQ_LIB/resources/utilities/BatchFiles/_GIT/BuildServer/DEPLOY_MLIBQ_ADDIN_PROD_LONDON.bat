@echo off

REM	Arg1 = x86 / x64 / ALL

SET MYCONFIG="%~1"
TITLE Deploying MLIBQ: Config %1 


REM Read Folder Control variables
SET DEPLOYMENT_FOLDER="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION"
SET DEPLOYMENT_FOLDER_X64="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION_X64"

REM Get Date- and Time-Stamp Information
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set DATESTAMP="%YYYY%-%MM%-%DD%"


REM CHECK FOR VALID CONFIG
IF %MYCONFIG%=="" (SET MYCONFIG="ALL")
IF %MYCONFIG%=="ALL" (GOTO :DEPLOY_X86)
IF %MYCONFIG%=="X86" (GOTO :DEPLOY_X86)
IF %MYCONFIG%=="X64" (GOTO :DEPLOY_X64)

:INVALID_CONFIG
ECHO.
ECHO. *** DEPLOY SCRIPT CANCELLED ***
ECHO. INVALID CONFIG: ARGUMENT 1 SHOULD BE SPECIFIED AS: "X86", X64", "ALL" OR LEFT BLANK = ALL
ECHO.
EXIT /B 1


REM THIS IS THE GOTO FOR ALL AND X86
:DEPLOY_X86


REM Print Copy Folder Diagnostic Information
echo DIAGNOSTIC INFORMATION X86
echo -------------------------------
echo.

echo Deploying MLIBQ Excel Add-In X86 (32-BIT)
echo.

REM Set Source & Version Here
set SOURCE="%MLIBQ%\target2017\32\Release\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"
Set VERSION="%DATESTAMP%"
Set CONFIG="%MLIBQ%\resource\config"

REM Set Folders Here
set VERSION_FOLDER="%DEPLOYMENT_FOLDER%\%VERSION%"

REM Create a folder with today's date and version
call %~dp0\UpdateFolderVersion.bat "%VERSION_FOLDER%"
IF not exist "%NEW_FOLDER_PATH%" (mkdir "%NEW_FOLDER_PATH%")
IF not exist "%NEW_FOLDER_PATH%\config\" (mkdir "%NEW_FOLDER_PATH%\config\")
echo.

echo Destination Folder:
echo "%NEW_FOLDER_PATH%"
echo.

echo Excel Add-In:
echo "%SOURCE%"
echo.

echo Config File Folder:
echo "%CONFIG%"
echo.

echo ----------------------
echo.

echo COPYING EXCEL ADDIN
echo -------------------
REM Copy the Excel Add-In and Config Files to the new version folder
echo Copying "%SOURCE%" to "%NEW_FOLDER_PATH%\"
xcopy /y /i /r "%SOURCE%" "%NEW_FOLDER_PATH%\"
echo.

echo COPYING CONFIG
echo -------------------
echo Copying "%CONFIG%" to "%NEW_FOLDER_PATH%\"
xcopy /s /y /i /r "%CONFIG%" "%NEW_FOLDER_PATH%\config\"
echo.


REM ONLY CONTINUE IF CONFIG==ALL SELECTED
IF %MYCONFIG%=="ALL" (GOTO :DEPLOY_X64)
EXIT /B 0


REM THIS IS THE GOTO FOR ALL AND X86
:DEPLOY_X64


REM Print Copy Folder Diagnostic Information
echo DIAGNOSTIC INFORMATION X64
echo -------------------------------
echo.

echo Deploying MLIBQ Excel Add-In X64 (64-BIT)
echo.

REM Set Source & Version Here
set SOURCE="%MLIBQ%\target2017\64\Release\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"
Set VERSION="%DATESTAMP%"
Set CONFIG="%MLIBQ%\resource\config"

REM Set Folders Here
set VERSION_FOLDER="%DEPLOYMENT_FOLDER_X64%\%VERSION%"

REM Create a folder with today's date and version
call %~dp0\UpdateFolderVersion.bat "%VERSION_FOLDER%"
IF not exist "%NEW_FOLDER_PATH%" (mkdir "%NEW_FOLDER_PATH%")
REM IF not exist "%NEW_FOLDER_PATH%\config\" (mkdir "%NEW_FOLDER_PATH%\config\") <--- No need to copy config a second time (x86 and x64 share the same config)
echo.

echo Destination Folder:
echo "%NEW_FOLDER_PATH%"
echo.

echo Excel Add-In:
echo "%SOURCE%"
echo.

echo Config File Folder:
echo "%CONFIG%"
echo.

echo ----------------------
echo.

echo COPYING EXCEL ADDIN
echo -------------------
REM Copy the Excel Add-In and Config Files to the new version folder
echo Copying "%SOURCE%" to "%NEW_FOLDER_PATH%\"
xcopy /y /i /r "%SOURCE%" "%NEW_FOLDER_PATH%\"
echo.

echo COPYING CONFIG
echo -------------------
echo Copying "%CONFIG%" to "%NEW_FOLDER_PATH%\"
xcopy /s /y /i /r "%CONFIG%" "%NEW_FOLDER_PATH%\config\"
echo.

EXIT /B 0