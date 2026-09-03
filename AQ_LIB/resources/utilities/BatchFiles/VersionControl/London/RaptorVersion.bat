@echo off

REM SEARCH SWITCHES

REM /b 		= bare format
REM /ah-h	= only directories and -h means not the hidden ones
REM t:c 	= sort by creation date (use t:w to sort by the time of last write date)
REM /od 	= sort oldest first
REM FOR /F	= sets directory name to 

REM For more information see link https://stackoverflow.com/questions/10519389/get-last-created-directory-batch-command



REM SEARCH FOR LATEST RAPTOR RELEASE FOLDER
echo .
echo *** RAPTOR VERSION DIAGNOSTICS ***

REM SET RAPTOR PATH AND VERSION FOLDER PREFIX
set RAPTOR_PATH=\\fs002\Global\Secure\DIR_Raptor\RaptorExcelAddin\
echo RAPTOR PATH: %RAPTOR_PATH%

REM SET THE RAPTOR VERSION FOLDER PREFIX, WHICH REPRESENTS THE RELEASE YEAR YYYY
set FOLDER_PREFIX=20*
echo FOLDER PREFIX: %FOLDER_PREFIX%

REM SEARCH RAPTOR PATH FOR LATEST RAPTOR RELEASE FOLDER USING THE VERSION FOLDER PREFIX
FOR /F "delims=" %%i IN ('dir %RAPTOR_PATH%%FOLDER_PREFIX% /b /ah-h /t:c /od') DO SET RAPTOR_SUB_FOLDER=%%~ni

SET RAPTOR_FOLDER=%RAPTOR_PATH%%RAPTOR_SUB_FOLDER%
echo RAPTOR FOLDER: %RAPTOR_FOLDER%
echo .

REM RAPTOR VERSION
set RAPTOR_X64="%RAPTOR_FOLDER%\RaptorXlAddin64-PRD.xll"
set RAPTOR_X86="%RAPTOR_FOLDER%\RaptorXlAddin-PRD.xll"

