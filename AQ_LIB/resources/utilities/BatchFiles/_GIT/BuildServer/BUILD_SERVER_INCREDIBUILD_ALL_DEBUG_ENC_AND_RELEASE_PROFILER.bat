@echo off

TITLE Build MLIBQ: INCREDIBUILD BUILD ALL (INCL. DEBUG ENC AND RELEASE PROFILER)

REM Read Folder Control variables
IF NOT DEFINED INCREDIBUILD (SET INCREDIBUILD="C:\Program Files (x86)\Xoreax\IncrediBuild")

REM Check IncrediBuild Installed
IF not exist "%INCREDIBUILD%" (
ECHO .
ECHO INCREDIBUILD NOT FOUND
ECHO Please ensure IncrediBuild is installed in "C:\Program Files (x86)\Xoreax\IncrediBuild"
ECHO or set the INCREDIBUILD Environment Variable if IncredibBuild has been custom installed elsewhere.
ECHO .
PAUSE
EXIT )

REM -------------------------------------------------------------------------------------------
ECHO.
ECHO *** BUILDING DEBUG/RELEASE WIN32 ***
ECHO.

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="Debug|x86"
ECHO *** DEBUG X86 OK ***

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="Release|x86"
ECHO *** RELEASE X86 OK ***

REM -------------------------------------------------------------------------------------------
ECHO.
ECHO *** BUILDING DEBUG, DEBUG_ENC, RELEASE AND RELEASE_PROFILER X64 ***
ECHO.

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="Debug|x64"
ECHO *** DEBUG X64 OK ***

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="DebugEditAndContinue|x64"
ECHO *** DEBUG ENC X64 OK ***

CALL "%INCREDIBUILD%\BuildConsole.exe" %MLIBQ%\MLIBQ_2017.sln /build /cfg="Release|x64"
ECHO *** RELEASE X64 OK ***

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleaseProfiler|x64"
ECHO *** RELEASE PROFILER X64 OK ***

REM -------------------------------------------------------------------------------------------
REM BUILD MLIB APIs x64
ECHO.
ECHO *** BUILDING MLIB API: CSHARP, JAVA, PYTHON AND R X64 ***
ECHO.

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleaseCSharp|x64"
ECHO *** RELEASE CSHARP X64 OK ***

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleaseJava|x64"
ECHO *** RELEASE JAVA X64 OK ***

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleasePython|x64"
ECHO *** RELEASE PYTHON X64 OK ***

CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleaseR|x64"
ECHO *** RELEASE R X64 OK ***

rem Pause to Read Console
rem ---------------------
if "%1%"=="PAUSE" PAUSE