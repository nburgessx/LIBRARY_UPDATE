@echo off

TITLE Build MLIBQ: INCREDIBUILD BUILD ALL (INCL. DEBUG EDIT AND CONTINUE)

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
	
REM BUILD DEBUG / RELEASE WIN32
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="Debug|x86"
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="DebugEditAndContinue|x86"
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="Release|x86"

REM BUILD DEBUG / RELEASE x64
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="Debug|x64"
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="DebugEditAndContinue|x64"
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="Release|x64"

REM BUILD MLIB APIs x64
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleaseCSharp|x64"
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleaseJava|x64"
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleasePython|x64"
CALL "%INCREDIBUILD%\BuildConsole.exe" "%MLIBQ%\MLIBQ_2017.sln" /build /cfg="ReleaseR|x64"

rem Pause to Read Console
rem ---------------------
if "%1%"=="PAUSE" PAUSE