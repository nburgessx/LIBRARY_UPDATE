@echo off

TITLE Build MLIBQ using IncrediBuild

REM Read Folder Control variables
IF NOT DEFINED INCREDIBUILD (SET INCREDIBUILD="C:\Program Files (x86)\Xoreax\IncrediBuild")
	
REM	BUILD DEBUG / RELEASE WIN32
TITLE Build MLIBQ using IncrediBuild - DEBUG WIN32
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="Debug|Win32"
TITLE Build MLIB using IncrediBuild - RELEASE WIN32
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="Release|Win32"

REM BUILD DEBUG / RELEASE x64
TITLE Build MLIBQ using IncrediBuild - DEBUG
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="Debug|x64"
TITLE Build MLIBQ using IncrediBuild - RELEASE X64
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="Release|x64"

REM BUILD DEBUG VISUAL LEAK DETECTOR
TITLE Build MLIBQ using IncrediBuild - DEBUG VISUAL LEAK DETECETOR WIN32
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="DebugVisualLeakDetector|Win32"
TITLE Build MLIB using IncrediBuild - DEBUG VISUAL LEAK DETECETOR  X64
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="DebugVisualLeakDetector|x64"

REM BUILD MLIB APIs
TITLE Build MLIBQ using IncrediBuild - RELEASE C SHARP WIN32
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="ReleaseCSharp|Win32"
TITLE Build MLIBQ using IncrediBuild - RELEASE JAVA WIN32
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="ReleaseJava|Win32"
TITLE Build MLIBQ using IncrediBuild - RELEASE PYTHON WIN32
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="ReleasePython|Win32"
TITLE Build MLIBQ using IncrediBuild - RELEASE R WIN32
CALL %INCREDIBUILD%\BuildConsole.exe %MLIBQ%\MLIBQ_2015.sln /build /cfg="ReleaseR|Win32"

rem Pause to Read Console
rem ---------------------
if "%1%"=="PAUSE" PAUSE