echo off

set EXCEL="C:\Program Files (x86)\Microsoft Office\Office14\EXCEL.exe"
set MLIB="%~dp0\2017-06-02.5\etrading_addin.xll"
set WORKSHOP="%~dp0\Workshop1_GettingStarted.xlsm"
set MLIB_TOOLBAR="%~dp0\Toolbar\ExcelToolbar.xlam"

START "" %EXCEL% /e %MLIB% %WORKSHOP% %MLIB_TOOLBAR%
EXIT