echo off

set EXCEL="C:\Program Files (x86)\Microsoft Office\Office14\EXCEL.exe"
set MLIB="%~dp0\2016-10-07\etrading_addin.xll"
set WORKSHOP="%~dp0\Workshop0_YieldCurve&SwapPricing.xlsm"

START "" %EXCEL% /e %MLIB% %WORKSHOP%
EXIT