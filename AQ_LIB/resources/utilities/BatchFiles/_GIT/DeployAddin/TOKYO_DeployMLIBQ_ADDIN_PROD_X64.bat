@echo off

TITLE MLIBQ -- DEPLOY EXCEL ADDIN X64 -- TOKYO

REM Read Folder Control variables
IF NOT DEFINED FOLDERS_UPDATED CALL "%~dp0%\..\FolderControl\MLIBQ_FOLDERS_MASTER.bat"

REM Get Date- and Time-Stamp Information
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set DATESTAMP="%YYYY%-%MM%-%DD%"
set TIMESTAMP="%HH%%Min%%Sec%"
set FULLSTAMP="%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"


REM WINDOWS 10 x64 (64 BIT RELEASE)
REM ========================================================================================================

REM Set Source & Version Here
set SOURCE="%MLIBQ%\target2017\64\Release\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"
Set VERSION="%DATESTAMP%"
Set CONFIG="%MLIBQ%\resource\config"

REM Set Folders Here
set LOCAL="%BATCH_FILE_FOLDER%\DeployAddin\MLIBQ_ADDIN"
set LOCAL_CONFIG="%BATCH_FILE_FOLDER%\DeployAddin\MLIBQ_ADDIN\config"
set PRODUCTION="%DEPLOYMENT_FOLDER_TOKYO_MLIBQ_PRODUCTION_X64%\%VERSION%"
set UAT="%DEPLOYMENT_FOLDER_TOKYO_MLIBQ_UAT_X64%\%VERSION%"

REM Copy Source Add-In to the Local Add-In Folder
xcopy /y "%SOURCE%" "%LOCAL%\"
xcopy /s /y "%CONFIG%" "%LOCAL_CONFIG%\"

REM Create a folder with today's date and version then copy the Excel Add-In and Config Files to that path
REM call %~dp0\UpdateFolderVersion.bat "%UAT%"
REM IF not exist "%NEW_FOLDER_PATH%" (mkdir "%NEW_FOLDER_PATH%")
REM xcopy /s /y "%LOCAL%" "%NEW_FOLDER_PATH%\"

REM Create a folder with today's date and version then copy the Excel Add-In and Config Files to that path
call %~dp0\UpdateFolderVersion.bat "%PRODUCTION%"
IF not exist "%NEW_FOLDER_PATH%" (mkdir "%NEW_FOLDER_PATH%")
xcopy /s /y "%LOCAL%" "%NEW_FOLDER_PATH%\"