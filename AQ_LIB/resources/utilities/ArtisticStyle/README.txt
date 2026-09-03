Artistic Stlye (Astyle)
-----------------------
This tool will format C++ code based on a series of commands, see the below web-link
for a full list of commands and options.

Weblink
http://astyle.sourceforge.net/astyle.html


Running the Styler
------------------
The styler can be run on command line or as a batch file. See below example.

A batch file has been set-up to format the following projects, which can be found in
%MLIB%\ArtisticStlye\bin folder see FormatEtrading.bat

etrading
ETRADING_ADDIN
validation_api
GOOGLE_TEST


Example
-------
@echo Formating Project: Etrading
@echo.
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\etrading\src\*.cpp
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\etrading\include\*.h

@echo Formating Project: Etrading Addin
@echo.
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\etrading_addin\src\*.cpp
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\etrading_addin\include\*.h

@echo Formating Project: Google Test
@echo.
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\GOOGLE_TEST\src\*.cpp
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\GOOGLE_TEST\include\*.h

@echo Formating Project: Validation API
@echo.
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\validation_api\src\*.cpp
AStyle.exe -A1 -S -N -w -Y -M -p -k1 -y -e -D --recursive %MLIB%\mlib\src\validation_api\include\*.h
