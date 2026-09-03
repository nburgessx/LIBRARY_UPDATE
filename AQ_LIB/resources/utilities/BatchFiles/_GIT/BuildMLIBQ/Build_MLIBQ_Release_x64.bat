@echo off

TITLE Building MLIBQ Release x64

REM Read Folder Control variables
IF NOT DEFINED FOLDERS_UPDATED CALL %~dp0\..\FolderControl\MLIBQ_FOLDERS_MASTER.bat

CALL %VISUAL_STUDIO_MSBUILD_PATH%\msbuild.exe %MLIBQ%\MLIBQ_2015.sln /p:Configuration=release /p:Platform=x64 /p:VCTargetsPath=%VISUAL_STUDIO_TARGETS_PATH% /nr:false /m /fl3 /flp3:LogFile=%LOG_FILES_FOLDER%\Release64.log;Verbosity=diagnostic 

rem Pause to Read Console
rem ---------------------
if "%1%"=="PAUSE" PAUSE
