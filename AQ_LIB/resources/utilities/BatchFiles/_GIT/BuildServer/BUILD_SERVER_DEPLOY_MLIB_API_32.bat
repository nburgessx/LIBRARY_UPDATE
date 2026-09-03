@echo off


TITLE *** SET API CONTROL VARIABLES ***
REM ============================================================================================

REM Configuration Folder Settings
Set CONFIG="%MLIBQ%\resource\config"
Set CONFIG_CALENDAR="%MLIBQ%\resource\config\Calendar.csv"
Set CONFIG_PROPERTYFILE="%MLIBQ%\resource\config\ir.properties"

REM Excel Folder Settings
set EXCEL_DEBUG_32_SOURCE="%MLIBQ%\target2017\32\Debug\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"
set EXCEL_RELEASE_32_SOURCE="%MLIBQ%\target2017\32\Release\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"

set EXCEL_DEBUG_64_SOURCE="%MLIBQ%\target2017\64\Debug\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"
set EXCEL_RELEASE_64_SOURCE="%MLIBQ%\target2017\64\Release\MLIBQ_ADDIN\out\MLIBQ_ADDIN.xll"

set EXCEL_FOLDER_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Excel"
set EXCEL_DEBUG_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Excel\Debug"
set EXCEL_RELEASE_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Excel\Release"

REM CSharp Folder Settings
set CSHARP_DLL_RELEASE_32_SOURCE="%MLIBQ%\target2017\32\ReleaseCSharp\MLIB_CLIENT_API\out\DLL"
set CSHARP_DLL_RELEASE_64_SOURCE="%MLIBQ%\target2017\64\ReleaseCSharp\MLIB_CLIENT_API\out\DLL"
set CSHARP_GENERATEDFILES_32_SOURCE="%MLIBQ%\target2017\32\ReleaseCSharp\MLIB_CLIENT_API\out\GeneratedFiles"
set CSHARP_GENERATEDFILES_64_SOURCE="%MLIBQ%\target2017\64\ReleaseCSharp\MLIB_CLIENT_API\out\GeneratedFiles"

set CSHARP_FOLDER_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\CSharp"
set CSHARP_DLL_FOLDER_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\CSharp\DLL"
set CSHARP_DLL_DEBUG_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\CSharp\DLL\Debug"
set CSHARP_DLL_RELEASE_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\CSharp\DLL\Release"
set CSHARP_GENERATEDFILES_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\CSharp\GeneratedFiles"

REM Java Folder Settings
set JAVA_DLL_RELEASE_32_SOURCE="%MLIBQ%\target2017\32\ReleaseJava\MLIB_CLIENT_API\out\DLL"
set JAVA_DLL_RELEASE_64_SOURCE="%MLIBQ%\target2017\64\ReleaseJava\MLIB_CLIENT_API\out\DLL"
set JAVA_GENERATEDFILES_32_SOURCE="%MLIBQ%\target2017\32\ReleaseJava\MLIB_CLIENT_API\out\GeneratedFiles"
set JAVA_GENERATEDFILES_64_SOURCE="%MLIBQ%\target2017\64\ReleaseJava\MLIB_CLIENT_API\out\GeneratedFiles"

set JAVA_FOLDER_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Java"
set JAVA_DLL_FOLDER_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Java\DLL"
set JAVA_DLL_DEBUG_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Java\DLL\Debug"
set JAVA_DLL_RELEASE_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Java\DLL\Release"
set JAVA_GENERATEDFILES_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Java\GeneratedFiles"

REM Python Folder Settings
set PYTHON_DLL_RELEASE_32_SOURCE="%MLIBQ%\target2017\32\ReleasePython\MLIB_CLIENT_API\out\DLL"
set PYTHON_DLL_RELEASE_64_SOURCE="%MLIBQ%\target2017\64\ReleasePython\MLIB_CLIENT_API\out\DLL"
set PYTHON_GENERATEDFILES_32_SOURCE="%MLIBQ%\target2017\32\ReleasePython\MLIB_CLIENT_API\out\GeneratedFiles"
set PYTHON_GENERATEDFILES_64_SOURCE="%MLIBQ%\target2017\64\ReleasePython\MLIB_CLIENT_API\out\GeneratedFiles"

set PYTHON_FOLDER_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Python"
set PYTHON_DLL_FOLDER_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Python\DLL"
set PYTHON_DLL_DEBUG_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Python\DLL\Debug"
set PYTHON_DLL_RELEASE_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Python\DLL\Release"
set PYTHON_GENERATEDFILES_COPY="%MLIBQ%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\Python\GeneratedFiles"

REM R Folder Settings
set R_DLL_RELEASE_32_SOURCE="%MLIB%\target2017\32\ReleaseR\MLIB_CLIENT_API\out\DLL"
set R_DLL_RELEASE_64_SOURCE="%MLIB%\target2017\64\ReleaseR\MLIB_CLIENT_API\out\DLL"
set R_GENERATEDFILES_32_SOURCE="%MLIB%\target2017\32\ReleaseR\MLIB_CLIENT_API\out\GeneratedFiles"
set R_GENERATEDFILES_64_SOURCE="%MLIB%\target2017\64\ReleaseR\MLIB_CLIENT_API\out\GeneratedFiles"

set R_FOLDER_COPY="%MLIB%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\R"
set R_DLL_FOLDER_COPY="%MLIB%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\R\DLL"
set R_DLL_DEBUG_COPY="%MLIB%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\R\DLL\Debug"
set R_DLL_RELEASE_COPY="%MLIB%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\R\DLL\Release"
set R_GENERATEDFILES_COPY="%MLIB%\resource\utilities\BatchFiles\_GIT\BuildServer\DeployAPI\R\GeneratedFiles"


TITLE *** SET VERSION VARIABLE USING DATE AND TIMESTAMP ***
REM ============================================================================================


REM Get Date- and Time-Stamp Information
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set DATESTAMP="%YYYY%-%MM%-%DD%"
set TIMESTAMP="%HH%%Min%%Sec%"
set FULLSTAMP="%YYYY%-%MM%-%DD%_%HH%-%Min%-%Sec%"

REM Set Source & Version Here
REM set SOURCE="%MLIB%\target2017\32\Release\ETRADING_ADDIN\out\etrading_addin.xll"
Set VERSION="%DATESTAMP%"
REM Set CONFIG="%MLIB%\resource\config"


TITLE *** CREATE EXCEL API PACKAGE***
REM ============================================================================================


REM Copy Source Files to the Local Copy of the Deployment Folder
REM Note We Clear the Copy Folder First, Suppressing Any Command Output Using ">nul 2>&1"
RD /S /Q "%EXCEL_FOLDER_COPY%" >nul 2>&1

MD "%EXCEL_FOLDER_COPY%"
REM MD "%EXCEL_DEBUG_COPY%"
MD "%EXCEL_RELEASE_COPY%"

REM Copy Source Add-In to the Local Add-In Folder
Set CONFIG="%MLIBQ%\resource\config"

REM xcopy /s /y "%EXCEL_DEBUG_SOURCE%" "%EXCEL_DEBUG_COPY%"
REM xcopy /s /y "%CONFIG%" "%EXCEL_DEBUG_COPY%\config\"

xcopy /s /y "%EXCEL_RELEASE_32_SOURCE%" "%EXCEL_RELEASE_COPY%"
xcopy /s /y "%CONFIG%" "%EXCEL_RELEASE_COPY%\config\"


TITLE *** CREATE CSHARP API PACKAGE ***
REM ============================================================================================


REM Copy Source Files to the Local Copy of the Deployment Folder
REM Note We Clear the Copy Folder First, Suppressing Any Command Output Using ">nul 2>&1"
RD /S /Q "%CSHARP_FOLDER_COPY%" >nul 2>&1

MD "%CSHARP_FOLDER_COPY%"
MD "%CSHARP_DLL_FOLDER_COPY%"
REM MD "%CSHARP_DLL_DEBUG_COPY%"
MD "%CSHARP_DLL_RELEASE_COPY%"
MD "%CSHARP_GENERATEDFILES_COPY%"

REM xcopy /s /y "%CSHARP_DLL_DEBUG_SOURCE%" "%CSHARP_DLL_DEBUG_COPY%"
xcopy /s /y "%CSHARP_DLL_RELEASE_32_SOURCE%" "%CSHARP_DLL_RELEASE_COPY%"
xcopy /s /y "%CSHARP_GENERATEDFILES_32_SOURCE%" "%CSHARP_GENERATEDFILES_COPY%"


TITLE *** CREATE JAVA API PACKAGE ***
REM ============================================================================================


REM Copy Source Files to the Local Copy of the Deployment Folder
REM Note We Clear the Copy Folder First, Suppressing Any Command Output Using ">nul 2>&1"
RD /S /Q "%JAVA_FOLDER_COPY%" >nul 2>&1

MD "%JAVA_FOLDER_COPY%"
MD "%JAVA_DLL_FOLDER_COPY%"
REM MD "%JAVA_DLL_DEBUG_COPY%"
MD "%JAVA_DLL_RELEASE_COPY%"
MD "%JAVA_GENERATEDFILES_COPY%"

REM xcopy /s /y "%JAVA_DLL_DEBUG_SOURCE%" "%JAVA_DLL_DEBUG_COPY%"
xcopy /s /y "%JAVA_DLL_RELEASE_32_SOURCE%" "%JAVA_DLL_RELEASE_COPY%"
xcopy /s /y "%JAVA_GENERATEDFILES_32_SOURCE%" "%JAVA_GENERATEDFILES_COPY%"


TITLE *** CREATE PYTHON API PACKAGE ***
REM ============================================================================================


REM Copy Source Files to the Local Copy of the Deployment Folder
REM Note We Clear the Copy Folder First, Suppressing Any Command Output Using ">nul 2>&1"
RD /S /Q "%PYTHON_FOLDER_COPY%" >nul 2>&1

MD "%PYTHON_FOLDER_COPY%"
MD "%PYTHON_DLL_FOLDER_COPY%"
REM MD "%PYTHON_DLL_DEBUG_COPY%"
MD "%PYTHON_DLL_RELEASE_COPY%"
MD "%PYTHON_GENERATEDFILES_COPY%"

REM xcopy /s /y "%PYTHON_DLL_DEBUG_SOURCE%" "%PYTHON_DLL_DEBUG_COPY%"
xcopy /s /y "%PYTHON_DLL_RELEASE_32_SOURCE%" "%PYTHON_DLL_RELEASE_COPY%"
xcopy /s /y "%PYTHON_GENERATEDFILES_32_SOURCE%" "%PYTHON_GENERATEDFILES_COPY%"


TITLE *** CREATE R API PACKAGE ***
REM ============================================================================================


REM Copy Source Files to the Local Copy of the Deployment Folder
REM Note We Clear the Copy Folder First, Suppressing Any Command Output Using ">nul 2>&1"
RD /S /Q "%R_FOLDER_COPY%" >nul 2>&1

MD "%R_FOLDER_COPY%"
MD "%R_DLL_FOLDER_COPY%"
MD "%R_DLL_DEBUG_COPY%"
MD "%R_DLL_RELEASE_COPY%"
MD "%R_GENERATEDFILES_COPY%"

REM xcopy /s /y "%R_DLL_DEBUG_SOURCE%" "%R_DLL_DEBUG_COPY%"
xcopy /s /y "%R_DLL_RELEASE_32_SOURCE%" "%R_DLL_RELEASE_COPY%"
xcopy /s /y "%R_GENERATEDFILES_32_SOURCE%" "%R_GENERATEDFILES_COPY%"


REM CREATE DEPLOYMENT FOLDER
REM ============================================================================================

SET DEPLOYMENT_FOLDER_MLIB_CLIENT_API="\\fs002\Global\Secure\DIR_MLib_API"
SET ORIGINAL_FOLDER_PATH="%DEPLOYMENT_FOLDER_MLIB_CLIENT_API%/%DATESTAMP%"

REM CREATE A NEW FOLDER NAME AS THE ORIGINAL FOLDER WITH A SUFFIX FOR THE NEXT AVAILABLE VERSION NUMBER
REM E.G. COPYFILE -> COPYFILE.1
SET VERSION_COUNTER=0

REM WHILE LOOP UPDATE THE VERSION COUNTER TO THE NEXT FREE VERSION NUMBER
:WHILE_LOOP
SET /A VERSION_COUNTER=VERSION_COUNTER+1
SET NEW_FOLDER_PATH="%ORIGINAL_FOLDER_PATH%.%VERSION_COUNTER%"
IF EXIST "%NEW_FOLDER_PATH%" GOTO :WHILE_LOOP

REM REMOVE SURROUNDING QUOTES FROM THE FOLDER PATH
set NEW_FOLDER_PATH=%NEW_FOLDER_PATH:"=%

IF not exist "%NEW_FOLDER_PATH%" (mkdir "%NEW_FOLDER_PATH%")


REM *** DEPLOY API BINARIES ***
REM ============================================================================================

set NEW_FOLDER_PATH_WIN32=%NEW_FOLDER_PATH%\Win32
MD "%NEW_FOLDER_PATH_WIN32%\Excel"
MD "%NEW_FOLDER_PATH_WIN32%\CSharp"
MD "%NEW_FOLDER_PATH_WIN32%\Java"
MD "%NEW_FOLDER_PATH_WIN32%\Python"
MD "%NEW_FOLDER_PATH_WIN32%\R"

xcopy /s /y "%EXCEL_FOLDER_COPY%" "%NEW_FOLDER_PATH_WIN32%\Excel"
xcopy /s /y "%CSHARP_FOLDER_COPY%" "%NEW_FOLDER_PATH_WIN32%\Csharp"
xcopy /s /y "%JAVA_FOLDER_COPY%" "%NEW_FOLDER_PATH_WIN32%\Java"
xcopy /s /y "%PYTHON_FOLDER_COPY%" "%NEW_FOLDER_PATH_WIN32%\Python"
xcopy /s /y "%R_FOLDER_COPY%" "%NEW_FOLDER_PATH_WIN32%\R"

