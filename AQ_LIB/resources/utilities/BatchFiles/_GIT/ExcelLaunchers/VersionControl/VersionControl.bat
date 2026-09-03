@echo off

REM EXCEL_VERSIONS
set EXCEL_OFFICE2016_X64="C:\Program Files\Microsoft Office\Office16\EXCEL.exe"
set EXCEL_OFFICE2016_X86="C:\Program Files (x86)\Microsoft Office\Office16\EXCEL.exe"
set EXCEL_OFFICE2010_X64="C:\Program Files\Microsoft Office\Office14\EXCEL.exe"
set EXCEL_OFFICE2010_X86="C:\Program Files (x86)\Microsoft Office\Office14\EXCEL.exe"

REM SET THE EXCEL VERSION
if EXIST %EXCEL_OFFICE2016_X64% (
	set EXCEL=%EXCEL_OFFICE2016_X64%
	set IS_64_BIT=TRUE
	set ARCHITECTURE=_X64
	GOTO :EXCEL_SET
)

if EXIST %EXCEL_OFFICE2016_X86% (
	set EXCEL=%EXCEL_OFFICE2016_X86%
	set IS_64_BIT=FALSE
	set ARCHITECTURE
	GOTO :EXCEL_SET
)

if EXIST %EXCEL_OFFICE2010_X64% (
	set EXCEL=%EXCEL_OFFICE2010_X64%
	set IS_64_BIT=TRUE
	set ARCHITECTURE=_X64
	GOTO :EXCEL_SET
)

if EXIST %EXCEL_OFFICE2010_X86% (
	set EXCEL=%EXCEL_OFFICE2010_X86%
	set IS_64_BIT=FALSE
	set ARCHITECTURE
	GOTO :EXCEL_SET
)

:EXCEL_NOT_SET
echo .
echo Failed to Detect Excel Version
echo .
EXIT

:EXCEL_SET

echo .
echo EXCEL Configuration
echo -------------------
echo Excel Version: %EXCEL%
echo Using 64 Bit Excel: %IS_64_BIT%
echo .

REM MLIB VERSION CONTROL FOR UAT
set MLIB_CURRENT="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-11-06.1\MLIBQ_ADDIN.xll"
set MLIB_BACKUP="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-25.1\MLIBQ_ADDIN.xll"

echo %MLIB_CURRENT%

REM MLIB VERSION CONTROL FOR TRADING DESKS
set MLIB_AUD_BOND_DESK="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-04.1\MLIBQ_ADDIN.xll"
set MLIB_EUR_SWAPS_DESK="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-04.1\MLIBQ_ADDIN.xll"
set MLIB_USD_SWAPS_DESK="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-04.1\MLIBQ_ADDIN.xll"
set MLIB_JGB_DESK="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-04.1\MLIBQ_ADDIN.xll"

REM MLIB TOOLBAR VERSION CONTROL
set MLIB_TOOLBAR="\\fs002\Global\Secure\DIR_MLib_Analytics\Utilities\Toolbar\ExcelToolbar.xlam"

REM CURVE BUDDY VERSION CONTROL
set CURVE_BUDDY_MLIB_VERSION="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-08-01.1\MLIBQ_ADDIN.xll"
set CURVE_BUDDY_APP="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\CurveBuddy\bin\CurveBuilderTool.exe"

REM CURVE CALIBRATION TOOL
SET CURVE_BUILDER="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\CurveCalibrationTool\CurveCalibrationTool-12.9.xlsb"

REM SWAP PRICER VERSION CONTROL
set MLIB_SWAP_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-11-06.1\MLIBQ_ADDIN.xll"
set SWAP_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\SwapsPricer\SwapsPricerV2.3.xlsm"

REM BOND PRICER VERSION CONTROL
set BOND_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\BondPricer\BondPricer-0.2.xlsm"

REM USD PRICER VERSION CONTROL
set MLIB_USD_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-20.1\MLIBQ_ADDIN.xll"
set USD_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\USDPricer\USD Pricer 9.4.xlsm"

REM EUR PRICER VERSION CONTROL
set MLIB_EUR_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-20.1\MLIBQ_ADDIN.xll"
set EUR_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\EURPricer\EUR Pricer-4.1.xlsm"

REM GBP PRICER VERSION CONTROL
set MLIB_GBP_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-09-20.1\MLIBQ_ADDIN.xll"
set GBP_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\GBPPricer\GBP Pricer 7.8.xlsm"

REM EUR PRICER WITH GLOBAL CURVES VERSION CONTROL
set EUR_PRICER_GLOBAL_CURVES="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\EURPricer\EUR Pricer-3.0 - Global Curves.xlsm"

REM FX PRICER VERSION CONTROL
set FX_PRICER="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\FXPricer\FXPricer-0.1.xlsx"

REM XCCY_PRICER VERSION CONTROL
set MLIB_XCCY_DESK_PROD="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-03-27.1.1.1\MLIBQ_ADDIN.xll"
set XCCY_PRICER_PROD="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\XccyPricer\XccyPricerV7.3.xlsb"

set MLIB_XCCY_DESK_UAT="\\fs002\Global\Secure\DIR_MLib_Analytics\MLIB_Excel_Addin\MLIBQ_ADDIN\PRODUCTION%ARCHITECTURE%\2019-03-27.1.1.1\MLIBQ_ADDIN.xll"
set XCCY_PRICER_UAT="\\fs002\Global\Secure\DIR_MLib_Analytics\PricingTools\XccyPricer\XccyPricerV7.3.xlsb"
