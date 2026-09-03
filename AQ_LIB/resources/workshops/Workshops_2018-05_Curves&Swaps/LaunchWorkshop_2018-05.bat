echo off

set EXCEL="C:\Program Files (x86)\Microsoft Office\Office14\EXCEL.exe"
set MLIB="%~dp0\2018-05-21.2\etrading_addin.xll"
set WORKSHOP="%~dp0\Workshop_2018-05.xlsm"
set MLIB_TOOLBAR="%~dp0\Toolbar\ExcelToolbar.xlam"

START "" %EXCEL% /e %MLIB% %WORKSHOP% %MLIB_TOOLBAR%
EXIT