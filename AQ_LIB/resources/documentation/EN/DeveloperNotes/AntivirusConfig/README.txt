Antivirus Configuration
-----------------------

The following binaries are requied to be excluded from Antivirus behaviour monitoring.

This is to allow Visual Studio 2015 to function normally for C++; to allow the compiler
to generate binaries without file scanning, locking and blocking. This problem extends
itself to the Incredibuild component of Visual Studio also.

1)	Resolves Global Incredibuild Outages
	--------------------------------------------
	C:\Program Files (x86)\Xoreax\IncrediBuild\BuildSystem.exe
	C:\Program Files (x86)\Xoreax\IncrediBuild\CoordService.exe

2)      Resolves Compiler and Cross-Platform Outages & Performance Issues
	--------------------------------------------
	C:\Program Files (x86)\MSBuild\14.0\bin\MSBuild.exe

3)	Re-Enables the C++ Debugger & Profiler Features
	--------------------------------------------
	C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe
	C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\Remote Debugger\x64\msvsmon.exe
