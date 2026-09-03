@echo off

TITLE MLIBQ -- DEPLOY EXCEL ADDIN X86 AND X64 -- LONDON --- DEPLOYMENT OF X86 IN PROGRESS

REM Read Folder Control variables
IF NOT DEFINED FOLDERS_UPDATED CALL "%~dp0%\..\FolderControl\MLIBQ_FOLDERS_MASTER.bat"

REM Get Date- and Time-Stamp Information
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set DATESTAMP="%YYYY%-%MM%-%DD%"
set TIMESTAMP="%HH%%Min%%Sec%"
set FULLSTAMP="%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"


REM WINDOWS 7 x86 (32 BIT RELEASE)
REM ========================================================================================================

REM Set Excel AddIn & Version Here
set EXCEL_ADDIN="%MLIBQ%\target2017\32\Release\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"
Set VERSION="%DATESTAMP%"
Set CONFIG="%MLIBQ%\resource\config"

REM Set Folders Here
set PRODUCTION="%DEPLOYMENT_FOLDER_MLIBQ_PRODUCTION%\%VERSION%"
set UAT="%DEPLOYMENT_FOLDER_MLIBQ_UAT%\%VERSION%"

REM Create Deployment Folder
call %~dp0\UpdateFolderVersion.bat "%PRODUCTION%"
IF not exist "%NEW_FOLDER_PATH%" (mkdir "%NEW_FOLDER_PATH%")

REM Copy Excel AddIn and Config to Deployment Folder
xcopy /y "%EXCEL_ADDIN%" "%NEW_FOLDER_PATH%\"
xcopy /s /y "%CONFIG%" "%NEW_FOLDER_PATH%\Config\" 


REM WINDOWS 10 x64 (64 BIT RELEASE)
REM ========================================================================================================

TITLE MLIBQ -- DEPLOY EXCEL ADDIN X86 AND X64 -- LONDON --- DEPLOYMENT OF X64 IN PROGRESS

REM Set Excel AddIn & Version Here
set EXCEL_ADDIN="%MLIBQ%\target2017\64\Release\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"
Set VERSION="%DATESTAMP%"
Set CONFIG="%MLIBQ%\resource\config"

REM Set Folders Here
set PRODUCTION="%DEPLOYMENT_FOLDER_MLIBQ_PRODUCTION_X64%\%VERSION%"
set UAT="%DEPLOYMENT_FOLDER_MLIBQ_UAT_X64%\%VERSION%"

REM Create Deployment Folder
call %~dp0\UpdateFolderVersion.bat "%PRODUCTION%"
IF not exist "%NEW_FOLDER_PATH%" (mkdir "%NEW_FOLDER_PATH%")

REM Copy Excel AddIn and Config to Deployment Folder
xcopy /y "%EXCEL_ADDIN%" "%NEW_FOLDER_PATH%\"
xcopy /s /y "%CONFIG%" "%NEW_FOLDER_PATH%\Config\" 
