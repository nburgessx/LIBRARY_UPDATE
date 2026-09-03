@echo off

REM SVN PARAMETERS
REM ======================================
SET VISUAL_STUDIO_MSBUILD_PATH="C:\Windows\Microsoft.NET\Framework64\v4.0.30319"

REM NOTE THE FINAL SWITCH STATEMENT IS REQUIRD - DO NOT REMOVE THE FINAL '/' CHARACTER
SET VISUAL_STUDIO_TARGETS_PATH="C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\V140/"

REM MLIBQ PARAMETERS
REM ======================================
cd %MLIBQ%
SET EXTERNAL_LIBRARY_FOLDER=%CD%\MLIB_EXTERNAL_LIBS
SET RELEASE_NOTES_FOLDER=%MLIBQ%\resource\release_notes
SET LOG_FILES_FOLDER=%MLIBQ%\resource\Utilities\BatchFiles\_GIT\LogFiles

REM DEPLOYMENT FOLDER SETTINGS - LONDON SERVER
REM ==========================================
SET DEPLOYMENT_FOLDER_MLIBQ_PRODUCTION="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION"
SET DEPLOYMENT_FOLDER_MLIBQ_UAT="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\UAT"

SET DEPLOYMENT_FOLDER_VANILLA_PRODUCTION="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\vanilla_addin\PRODUCTION"
SET DEPLOYMENT_FOLDER_VANILLA_UAT="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\vanilla_addin\UAT"

REM DEPLOYMENT FOLDER SETTINGS - TOKYO SERVER
REM =========================================
SET DEPLOYMENT_FOLDER_TOKYO_MLIBQ_PRODUCTION="\\tkfsfsgbl01.topflagship.mizuho-sc.com\shrgbl\LN\ln_rstac\Fixed Income Trading\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION"
SET DEPLOYMENT_FOLDER_TOKYO_MLIBQ_UAT="\\tkfsfsgbl01.topflagship.mizuho-sc.com\shrgbl\LN\ln_rstac\Fixed Income Trading\MLIB_Excel_Addin\MLIBQ_ADDIN\UAT"

SET DEPLOYMENT_FOLDER_TOKYO_VANILLA_PRODUCTION="\\tkfsfsgbl01.topflagship.mizuho-sc.com\shrgbl\LN\ln_rstac\Fixed Income Trading\MLIB_Excel_Addin\vanilla_addin\PRODUCTION"
SET DEPLOYMENT_FOLDER_TOKYO_VANILLA_UAT="\\tkfsfsgbl01.topflagship.mizuho-sc.com\shrgbl\LN\ln_rstac\Fixed Income Trading\MLIB_Excel_Addin\vanilla_addin\UAT"