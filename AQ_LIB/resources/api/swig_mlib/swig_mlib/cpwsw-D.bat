@echo off

REM Copy to the Java console client
copy /y %MLIB%\api\swig_mlib\swig_mlib\*.java %MLIB%\api\client_apps\swig_mlib_java_main\src\main\java\com\mizuho\mlib
copy /y %MLIB%\api\swig_mlib\Debug\swig_mlib.dll %MLIB%\api\client_apps\swig_mlib_java_main

REM Copy to the Java GUI client
copy /y %MLIB%\api\swig_mlib\swig_mlib\*.java %MLIB%\api\client_apps\swig_mlib_java_swing\src\com\mizuho\mlib
copy /y %MLIB%\api\swig_mlib\Debug\swig_mlib.dll %MLIB%\api\client_apps\swig_mlib_java_swing
copy /y %MLIB%\api\swig_mlib\Debug\swig_mlib.dll %MLIB%\api\client_apps\swig_mlib_java_swing\dist
copy /y X:\Secure\DIR_MLib_API\Calendar.csv %MLIB%\api\client_apps\swig_mlib_java_swing\src\com\mizuho\mlib
copy /y X:\Secure\DIR_MLib_API\ir.properties %MLIB%\api\client_apps\swig_mlib_java_swing\src\com\mizuho\mlib

REM After running this rebuild your Java client

@echo on


