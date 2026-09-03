@echo off

REM Read Folder Control variables
SET DEPLOYMENT_FOLDER="\\tkfsfsgbl01.topflagship.mizuho-sc.com\shrgbl\LN\ln_rstac\Fixed Income Trading\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION"

REM Get Date- and Time-Stamp Information
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set DATESTAMP="%YYYY%-%MM%-%DD%"

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

REM Print Copy Folder Diagnostic Information
echo DIAGNOSTIC INFORMATION
echo ----------------------
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