@echo off

TITLE Formating Project: Etrading
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -j -y -D --recursive %MLIB%\mlib\src\etrading\src\*.cpp
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -j -y -D --recursive %MLIB%\mlib\src\etrading\include\*.h


TITLE Formatting Project: Google Test
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -j -y -D --recursive %MLIB%\mlib\src\GOOGLE_TEST\src\*.cpp
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -j -y -D --recursive %MLIB%\mlib\src\GOOGLE_TEST\include\*.h

TITLE Formatting Project: Validation API
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -j -y -D --recursive %MLIB%\mlib\src\validation_api\src\*.cpp
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -j -y -D --recursive %MLIB%\mlib\src\validation_api\include\*.h

REM TITLE Formatting Project: ir
REM AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\ir\src\*.cpp
REM AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\ir\include\*.h