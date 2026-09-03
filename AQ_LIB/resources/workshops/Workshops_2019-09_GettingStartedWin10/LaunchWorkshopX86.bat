echo off

set EXCEL="C:\Program Files (x86)\Microsoft Office\Office14\EXCEL.exe"
set MLIB="%~dp0\2019-09-02.1-X86\MLIBQ_ADDIN.xll"
set WORKSHOP="%~dp0\Workshop_GettingStartedWin10.xlsm"
set MLIB_TOOLBAR="%~dp0\Toolbar\ExcelToolbar.xlam"

START "" %EXCEL% /e %MLIB% %WORKSHOP% %MLIB_TOOLBAR%
EXIT