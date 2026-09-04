/*! @file
    @brief Risk Constant data declare file
*/
//  2007, AlgoQuantHub.

#ifndef AQLDefinitionsRisk_h
#define AQLDefinitionsRisk_h

#include "AQLDefinitions.h"


// DATA FOR RISK
#define AP_CALIBRATION_DATA_RISK_GRID   "RiskGrid"
#define AP_CALIBRATION_DATA_RISK_GRIDACTVALS   "RiskGridActVals"
#define AP_CALIBRATION_DATA_ISRISKENTITY   "isRiskEntity"
#define AP_CALIBRATION_DATA_ISPARALLELSHIFT   "isParallelShift"
#define AP_CALIBRATION_DATA_DELTATYPE   "DeltaType"
#define AP_CALIBRATION_DATA_VEGATYPE   "VegaType"
#define AP_CALIBRATION_DATA_RISK_TRADESETDATA   "RiskTradeSetAttr"

// Scenario Type
#define RISK_SCENARIO_VOL  "VOL"
#define RISK_SCENARIO_YIELD   "YIELD"
#define RISK_SCENARIO_YIELDMULTIPARALLEL   "YIELDMULTIPARALLEL"
#define RISK_SCENARIO_YIELDBASIS   "YIELDBASIS"
#define RISK_SCENARIO_YIELDZERO   "YIELDZERO"
#define RISK_SCENARIO_YIELDZEROBASIS   "YIELDZEROBASIS"
#define RISK_SCENARIO_FX   "FX"

// Bump
#define RISK_BUMPDIRECTION_UPSHIFT "UPSHIFT"
#define RISK_BUMPDIRECTION_DOWNSHIFT "DOWNSHIFT"
#define RISK_BUMPDIRECTION_UPDOWNSHIFT "UPDOWNSHIFT"

#define RISK_BUMPTYPE_GRID "GRID"
#define RISK_BUMPTYPE_WAVE "WAVE"

#define RISK_SHIFTTYPE_DIFF "DIFF"
#define RISK_SHIFTTYPE_RATIO "RATIO"

#define RISK_MARKET_BUMP "MARKET_PARAM"
#define RISK_MODEL_VOL_BUMP "MODEL_VOL"

// Grid type
#define RISK_GRID_MARKET "MARKET"
#define RISK_GRID_COORDINATES "COORDINATES"

// Market
#define RISK_MARKET_SWAPTION "SWAPTION"
#define RISK_MARKET_CAP "CAP"

#define RISK_VEGATYPE_ATM "ATM"
#define RISK_VEGATYPE_BETA "BETA"
#define RISK_VEGATYPE_RHO "RHO"
#define RISK_VEGATYPE_NU "NU"

// RISK
// OFFICIAL RISK
#define RISK_OFFICIAL_YIELD_DELTA "YIELDDELTA"
#define RISK_OFFICIAL_YIELD_BASIS "YIELDBASIS"
#define RISK_OFFICIAL_FX_DELTA "FXDELTA_OFFICIAL"
#define RISK_OFFICIAL_VOL_CAPVEGA "CAPVEGA"
#define RISK_OFFICIAL_VOL_SWAPTIONVEGA "SWAPTIONVEGA"
#define RISK_OFFICIAL_VOL_FXATMPARALLELVEGA "FXATMPARALLELVEGA"
#define RISK_OFFICIAL_YIELD_PARADELTA "YIELDPARADELTA"
#define RISK_OFFICIAL_YIELD_CREDITSPREADDELTA "CREDITSPREADDELTA"
#define RISK_OFFICIAL_THETA "THETA"
#define RISK_OFFICIAL_YIELD_IRDELTA "IRDELTA"

// FRONT RISK
#define RISK_FRONT_YIELD_IRDELTA "IRDELTA"
#define RISK_FRONT_YIELD_IRSHIFTDELTA "IRSHIFTDELTA"
#define RISK_FRONT_YIELD_BASISDELTA "BASISDELTA"
#define RISK_FRONT_VOL_IRVEGA "IRVEGA"
#define RISK_FRONT_VOL_IRSHIFTIRVEGA "IRSHIFTIRVEGA"
#define RISK_FRONT_FX_DELTA "FXDELTA_FRONT"
#define RISK_FRONT_VOL_FXVEGA "FXVEGA"
#define RISK_FRONT_FX_SHIFTDELTA "FXSHIFTDELTA"
#define RISK_FRONT_FXSHIFT_VOL_FXVEGA "FXSHIFTFXVEGA"
#define RISK_FRONT_FXSHIFT_YIELD_IRDELTA "FXSHIFTIRDELTA"
#define RISK_FRONT_FXSHIFT_YIELD_BASISDELTA "FXSHIFTBASISDELTA"
#define RISK_FRONT_FXVOLSHIFT_VOL_FXVEGA "FXVOLSHIFTFXVEGA"
#define RISK_FRONT_IRSHIFT_FX_DELTA "IRSHIFTFXDELTA"
#define RISK_FRONT_IRSHIFT_VOL_FXVEGA "IRSHIFTFXVEGA"
#define RISK_FRONT_OPTIONANALYTIC "OPTIONANALYTIC"
#define RISK_FRONT_COR_CORDELTA "CORDELTA"

// EQ RISK
#define RISK_EQ_DELTA "EQDELTA"
#define RISK_EQ_GAMMA "EQGAMMA"
#define RISK_EQ_ATM_VEGA "EQATMVEGA"
#define RISK_EQ_SKEW_VEGA "EQSKEWVEGA"
#define RISK_EQ_KURT_VEGA "EQKURTVEGA"
#define RISK_EQ_REPORATE_DELTA "EQREPORATEDELTA"
#define RISK_EQ_DIVIDEND_DELTA "EQDIVIDENDDELTA"

// common name
#define RISK_FX_DELTA "FXDELTA"

#define KEY_RISK_BASEYIELD_NAME "BASE_YIELD_NAME"
//********************************
//
// property key start
//
//********************************

#define RISK_KEY_CALIB_ISREALCALIB "risk.calib.isrealcalib"
#define RISK_KEY_OMIT_DIGITNUMBER "risk.omit.digitnumeber"
#define	RISK_KEY_ANALYTIC_ISANALYTIC "risk.analytic.isanalytic"
#define	RISK_KEY_ANALYTIC_CALCTYPE "risk.analytic.calctype"
//********************************
// Official Risk
//********************************

#define KEY_RISK_GRID_SAMETERM_EPS "risk.grid.sameterm.eps"


// IR DELTA
#define KEY_RISK_OFFICIAL_YIELD_DELTA_ISGRIDSENSITIVITY  "risk.official.yield.delta.isgridsensitivity"
#define KEY_RISK_OFFICIAL_YIELD_DELTA_ISPARALLEL  "risk.official.yield.delta.isparallel"
#define KEY_RISK_OFFICIAL_YIELD_DELTA_OPERATOR1  "risk.official.yield.delta.operator1"
#define KEY_RISK_OFFICIAL_YIELD_DELTA_OPERATOR2  "risk.official.yield.delta.operator2"
#define KEY_RISK_OFFICIAL_YIELD_DELTA_GRID_CALCBUFFER  "risk.official.yield.delta.grid.calcbuffer"
#define KEY_RISK_OFFICIAL_YIELD_DELTA_TARGET_CURRENCY "risk.official.yield.delta.target.currency"
#define KEY_RISK_OFFICIAL_YIELD_DELTA_CALIBRATION_TARGET_CURRENCY "risk.official.yield.delta.calibration.target.currency"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_COEFFICIENT1  ".risk.official.yield.delta.coefficient1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_COEFFICIENT2  ".risk.official.yield.delta.coefficient2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_PARALLEL_BPSHIFT1  ".risk.official.yield.delta.parallel.bpshift1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_PARALLEL_BPSHIFT2  ".risk.official.yield.delta.parallel.bpshift2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_GRID_BPSHIFT1  ".risk.official.yield.delta.grid.bpshift1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_GRID_BPSHIFT2  ".risk.official.yield.delta.grid.bpshift2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_TERM  ".risk.official.yield.delta.term"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_OUTPUT1  ".risk.official.yield.delta.output1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_OUTPUT2  ".risk.official.yield.delta.output2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_ISCOEFFICIENTRATIO1 ".risk.official.yield.delta.iscoefficientratio1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_ISCOEFFICIENTRATIO2 ".risk.official.yield.delta.iscoefficientratio2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_DELTA_ISRISKCURRENCYMODE ".risk.official.yield.delta.isriskcurrencymode"


// IR BASIS
#define KEY_RISK_OFFICIAL_YIELD_BASIS_ISGRIDSENSITIVITY  "risk.official.yield.basis.isgridsensitivity"
#define KEY_RISK_OFFICIAL_YIELD_BASIS_ISPARALLEL  "risk.official.yield.basis.isparallel"
#define KEY_RISK_OFFICIAL_YIELD_BASIS_OPERATOR1  "risk.official.yield.basis.operator1"
#define KEY_RISK_OFFICIAL_YIELD_BASIS_OPERATOR2  "risk.official.yield.basis.operator2"
#define KEY_RISK_OFFICIAL_YIELD_BASIS_GRID_CALCBUFFER  "risk.official.yield.basis.grid.calcbuffer"
#define KEY_RISK_OFFICIAL_YIELD_BASIS_TARGET_CURRENCY "risk.official.yield.basis.target.currency"
#define KEY_RISK_OFFICIAL_YIELD_BASIS_CALIBRATION_TARGET_CURRENCY "risk.official.yield.basis.calibration.target.currency"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_COEFFICIENT1  ".risk.official.yield.basis.coefficient1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_COEFFICIENT2  ".risk.official.yield.basis.coefficient2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_PARALLEL_BPSHIFT1  ".risk.official.yield.basis.parallel.bpshift1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_PARALLEL_BPSHIFT2  ".risk.official.yield.basis.parallel.bpshift2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_GRID_BPSHIFT1  ".risk.official.yield.basis.grid.bpshift1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_GRID_BPSHIFT2  ".risk.official.yield.basis.grid.bpshift2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_TERM  ".risk.official.yield.basis.term"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_OUTPUT1  ".risk.official.yield.basis.output1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_OUTPUT2  ".risk.official.yield.basis.output2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_ISCOEFFICIENTRATIO1 ".risk.official.yield.basis.iscoefficientratio1"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_ISCOEFFICIENTRATIO2 ".risk.official.yield.basis.iscoefficientratio2"
#define STATIC_DATA_KEY_RISK_OFFICIAL_YIELD_BASIS_ISRISKCURRENCYMODE ".risk.official.yield.basis.isriskcurrencymode"



// IR CAPVEGA
#define KEY_RISK_OFFICIAL_VOL_CAPVEGA_ISGRIDSENSITIVITY  "risk.official.vol.capvega.isgridsensitivity"
#define KEY_RISK_OFFICIAL_VOL_CAPVEGA_OPERATOR  "risk.official.vol.capvega.operator"
#define KEY_RISK_OFFICIAL_VOL_CAPVEGA_TARGET_CURRENCY "risk.official.vol.capvega.target.currency"
#define KEY_RISK_OFFICIAL_VOL_CAPVEGA_CALIBRATION_TARGET_CURRENCY "risk.official.vol.capvega.calibration.target.currency"
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_COEFFICIENT  ".risk.official.vol.capvega.coefficient"
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_OUTPUT  ".risk.official.vol.capvega.output"
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_CAPVEGA_SHIFTVAL  ".risk.official.vol.capvega.shiftval"

// IR SWAPTIONVEGA
#define KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_ISGRIDSENSITIVITY  "risk.official.vol.swaptionvega.isgridsensitivity"
#define KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_OPERATOR  "risk.official.vol.swaptionvega.operator"
#define KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_TARGET_CURRENCY "risk.official.vol.swaptionvega.target.currency"
#define KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_CALIBRATION_TARGET_CURRENCY "risk.official.vol.swaptionvega.calibration.target.currency"
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_COEFFICIENT  ".risk.official.vol.swaptionvega.coefficient"
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_OUTPUT  ".risk.official.vol.swaptionvega.output"
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_SHIFTVAL  ".risk.official.vol.swaptionvega.shiftval"
#define STATIC_DATA_KEY_RISK_OFFICIAL_VOL_SWAPTIONVEGA_ISRISKCURRENCYMODE  ".risk.official.vol.swaptionvega.isriskcurrencymode"

// FX DELTA
#define KEY_RISK_OFFICIAL_FX_DELTA_TARGET_FX  "risk.official.fx.delta.target.fx"
#define KEY_RISK_OFFICIAL_FX_DELTA_CALIBRATION_TARGET_FX  "risk.official.fx.delta.calibration.target.fx"
#define KEY_RISK_OFFICIAL_FX_DELTA_ISGRIDSENSITIVITY  "risk.official.fx.delta.isgridsensitivity"
#define KEY_RISK_OFFICIAL_FX_DELTA_OPERATOR1  "risk.official.fx.delta.operator1"
#define KEY_RISK_OFFICIAL_FX_DELTA_OPERATOR2  "risk.official.fx.delta.operator2"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_COEFFICIENT1  ".risk.official.fx.delta.coefficient1"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_COEFFICIENT2  ".risk.official.fx.delta.coefficient2"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_OUTPUT1  ".risk.official.fx.delta.output1"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_OUTPUT2  ".risk.official.fx.delta.output2"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_SHIFT1  ".risk.official.fx.delta.shift1"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_SHIFT2  ".risk.official.fx.delta.shift2"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_SHIFTTYPE ".risk.official.fx.delta.shifttype"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_ISCOEFFICIENTRATIO1 ".risk.official.fx.delta.iscoefficientratio1"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_ISCOEFFICIENTRATIO2 ".risk.official.fx.delta.iscoefficientratio2"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_ISSPOTADJUSTMODEFORGAMMA ".risk.official.fx.delta.isspotadjustmodeforgamma"
#define FX_KEY_RISK_OFFICIAL_FX_DELTA_ISRISKCURRENCYMODE ".risk.official.fx.delta.isriskcurrencymode"


// FX ATM PARALLELVEGA
#define KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_ISGRIDSENSITIVITY  "risk.official.vol.fxatmparallelvega.isgridsensitivity"
#define KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OPERATOR  "risk.official.vol.fxatmparallelvega.operator"
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_COEFFICIENT  ".risk.official.vol.fxatmparallelvega.coefficient"
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_OUTPUT  ".risk.official.vol.fxatmparallelvega.output"
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_SHIFTVAL  ".risk.official.vol.fxatmparallelvega.shiftval"
#define FX_KEY_RISK_OFFICIAL_VOL_ATMPARALLELVEGA_ISRISKCURRENCYMODE ".risk.official.vol.fxatmparallelvega.isriskcurrencymode"

//CREDITSPREADDELTA
#define RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_BUMPDIRECTION "risk.official.yield.creditspreaddelta.bumpdirection"
#define RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_SHIFTVAL "risk.official.yield.creditspreaddelta.shiftval"
#define RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_SHIFTVAL_ISTIMERATIO "risk.official.yield.creditspreaddelta.shiftval.istimeratio"
#define RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_OUTPUT "risk.official.yield.creditspreaddelta.output"
#define RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_DIVUNIT "risk.official.yield.creditspreaddelta.divunit"
#define RISK_OFFICIAL_YIELD_CREDITSPREADDELTA_ISLOOKUPFNDNGSPDATT "risk.official.yield.creditspreaddelta.islookupfndngspdatt"

//********************************
// Front Risk
//********************************
// IR DELTA
#define RISK_FRONT_YIELD_IRDELTA_TARGET_CURRENCY  "risk.front.yield.irdelta.target.currency"
#define RISK_FRONT_YIELD_IRDELTA_CALIBRATION_TARGET_CURRENCY  "risk.front.yield.irdelta.calibration.target.currency"
#define RISK_FRONT_YIELD_IRDELTA_GRID_CALCBUFFER  "risk.front.yield.irdelta.grid.calcbuffer"
#define RISK_FRONT_YIELD_IRDELTA_ISSIMPLECALC "risk.front.yield.irdelta.issimplecalc"
#define RISK_FRONT_YIELD_IRDELTA_ISCALCGAMMA "risk.front.yield.irdelta.iscalcgamma"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OUTPUTNAME  ".risk.front.yield.irdelta.output"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OUTPUTNAME2  ".risk.front.yield.irdelta.output2"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISGRIDSENSITIVITY  ".risk.front.yield.irdelta.isgridsensitivity"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISPARALLEL  ".risk.front.yield.irdelta.isparallel"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_SHIFTTYPE  ".risk.front.yield.irdelta.shifttype"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_PARALLEL_SHIFTVAL  ".risk.front.yield.irdelta.parallel.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_GRID_TERM  ".risk.front.yield.irdelta.grid.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUCKET_TERM  ".risk.front.yield.irdelta.bucket.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_GRID_SHIFTVAL  ".risk.front.yield.irdelta.grid.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_GRID_ISTERMCHECK  ".risk.front.yield.irdelta.grid.istermcheck"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_BUMPDIRECTION  ".risk.front.yield.irdelta.bumpdirection"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISWAVE  ".risk.front.yield.irdelta.iswave"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_DIVUNIT  ".risk.front.yield.irdelta.divunit"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISRISKCURRENCYMODE ".risk.front.yield.irdelta.isriskcurrencymode"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISZERORATEBUMP ".risk.front.yield.irdelta.iszeroratebump"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_OMITNOTIONALEXPOSURE ".risk.front.yield.irdelta.omitnotionalexposure"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_ISIMMFWDRATEBUMP ".risk.front.yield.irdelta.isimmfwdratebump"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_IMMFWDRATETERM ".risk.front.yield.irdelta.immfwdrateterm"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_RECALCBASISDFSONZERORATEBUMP ".risk.front.yield.irdelta.recalcbasisdfsonzeroratebump"

// IR VEGA
#define RISK_FRONT_VOL_IRVEGA_TARGET_CURRENCY  "risk.front.vol.irvega.target.currency"
#define RISK_FRONT_VOL_IRVEGA_CALIBRATION_TARGET_FX  "risk.front.vol.irvega.calibration.target.fx"
#define RISK_FRONT_VOL_IRVEGA_CALIBRATION_TARGET_CURRENCY  "risk.front.vol.irvega.calibration.target.currency"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_OUTPUTNAME  ".risk.front.vol.irvega.output"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTTYPE  ".risk.front.vol.irvega.shifttype"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISGRIDSENSITIVITY  ".risk.front.vol.irvega.isgridsensitivity"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISPARALLEL  ".risk.front.vol.irvega.isparallel"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_TYPE  ".risk.front.vol.irvega.grid.type"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_MARKET  ".risk.front.vol.irvega.grid.market"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_GRID_COORDINATES  ".risk.front.vol.irvega.grid.coordinates"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_BUMPDIRECTION  ".risk.front.vol.irvega.bumpdirection"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISFILEUSE  ".risk.front.vol.irvega.isfileuse"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTVAL  ".risk.front.vol.irvega.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_SHIFTVAL_FILE  ".risk.front.vol.irvega.shiftval.file"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_DIVUNIT  ".risk.front.vol.irvega.divunit"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISWAVE  ".risk.front.vol.irvega.iswave"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISRISKCURRENCYMODE ".risk.front.vol.irvega.isriskcurrencymode"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_VEGATYPE ".risk.front.vol.irvega.vegatype"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_ISCONVERTED ".risk.front.vol.irvega.isconverted"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDFORWARDSHIFTVALUE ".risk.front.vol.irvega.convertedforwardshiftvalue"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDUNDERLYING ".risk.front.vol.irvega.convertedunderlying"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDCAPLETTENOR ".risk.front.vol.irvega.convertedcaplettenor"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDRATIOUPPERBOUND ".risk.front.vol.irvega.convertedratioupperbound"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRVEGA_CONVERTEDRATIOLOWERBOUND ".risk.front.vol.irvega.convertedratiolowerbound"
#define STATIC_DATA_KEY_VOL_UNDERLYING ".vol.underlying"

// IR SHIFT VEGA
#define RISK_FRONT_VOL_IRSHIFTIRVEGA_TARGET_CURRENCY  "risk.front.vol.irshiftirvega.target.currency"
#define RISK_FRONT_VOL_IRSHIFTIRVEGA_CALIBRATION_TARGET_FX  "risk.front.vol.irshiftirvega.calibration.target.fx"
#define RISK_FRONT_VOL_IRSHIFTIRVEGA_CALIBRATION_TARGET_CURRENCY  "risk.front.vol.irshiftirvega.calibration.target.currency"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_OUTPUTNAME  ".risk.front.vol.irshiftirvega.output"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTTYPE  ".risk.front.vol.irshiftirvega.shifttype"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISGRIDSENSITIVITY  ".risk.front.vol.irshiftirvega.isgridsensitivity"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISPARALLEL  ".risk.front.vol.irshiftirvega.isparallel"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TYPE  ".risk.front.vol.irshiftirvega.grid.type"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_MARKET  ".risk.front.vol.irshiftirvega.grid.market"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_COORDINATES  ".risk.front.vol.irshiftirvega.grid.coordinates"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_BUMPDIRECTION  ".risk.front.vol.irshiftirvega.bumpdirection"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISFILEUSE  ".risk.front.vol.irshiftirvega.isfileuse"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTVAL  ".risk.front.vol.irshiftirvega.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_SHIFTVAL_FILE  ".risk.front.vol.irshiftirvega.shiftval.file"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_DIVUNIT  ".risk.front.vol.irshiftirvega.divunit"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISWAVE  ".risk.front.vol.irshiftirvega.iswave"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_ISRISKCURRENCYMODE ".risk.front.vol.irshiftirvega.isriskcurrencymode"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_BASESHIFTVAL  ".risk.front.vol.irshiftirvega.baseshiftval"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_GRID_TERM  ".risk.front.vol.irshiftirvega.grid.term"
#define STATIC_DATA_KEY_RISK_FRONT_VOL_IRSHIFTIRVEGA_BASESHIFTCURVETYPE  ".risk.front.vol.irshiftirvega.baseshiftcurvetype"

// IR SHIFT DELTA
#define RISK_FRONT_YIELD_IRSHIFTDELTA_TARGET_CURRENCY  "risk.front.yield.irshiftdelta.target.currency"
#define RISK_FRONT_YIELD_IRSHIFTDELTA_CALIBRATION_TARGET_CURRENCY "risk.front.yield.irshiftdelta.calibration.target.currency"
#define RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_CALCBUFFER  "risk.front.yield.irshiftdelta.grid.calcbuffer"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_OUTPUTNAME  ".risk.front.yield.irshiftdelta.output"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISGRIDSENSITIVITY  ".risk.front.yield.irshiftdelta.isgridsensitivity"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISPARALLEL  ".risk.front.yield.irshiftdelta.isparallel"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SHIFTTYPE  ".risk.front.yield.irshiftdelta.shifttype"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_PARALLEL_SHIFTVAL  ".risk.front.yield.irshiftdelta.parallel.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_TERM  ".risk.front.yield.irshiftdelta.grid.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUCKET_TERM  ".risk.front.yield.irshiftdelta.bucket.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_SHIFTVAL  ".risk.front.yield.irshiftdelta.grid.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_GRID_ISTERMCHECK  ".risk.front.yield.irshiftdelta.grid.istermcheck"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_BUMPDIRECTION  ".risk.front.yield.irshiftdelta.bumpdirection"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISWAVE  ".risk.front.yield.irshiftdelta.iswave"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_DIVUNIT  ".risk.front.yield.irshiftdelta.divunit"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_IRSHIFTVAL  ".risk.front.yield.irshiftdelta.irshiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISIRSHIFTSCENARIO  ".risk.front.yield.irshiftdelta.isirshiftscenario"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_SCENARIO_IRSHIFTVAL  ".risk.front.yield.irshiftdelta.scenario.irshiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISRISKCURRENCYMODE ".risk.front.yield.irshiftdelta.isriskcurrencymode"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_IRSHIFTDELTA_ISZERORATEBUMP ".risk.front.yield.irshiftdelta.iszeroratebump"

// BASIS DELTA
#define RISK_FRONT_YIELD_BASISDELTA_TARGET_CURRENCY  "risk.front.yield.basisdelta.target.currency"
#define RISK_FRONT_YIELD_BASISDELTA_CALIBRATION_TARGET_CURRENCY  "risk.front.yield.basisdelta.calibration.target.currency"
#define RISK_FRONT_YIELD_BASISDELTA_GRID_CALCBUFFER  "risk.front.yield.basisdelta.grid.calcbuffer"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OUTPUTNAME  ".risk.front.yield.basisdelta.output"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISGRIDSENSITIVITY  ".risk.front.yield.basisdelta.isgridsensitivity"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISPARALLEL  ".risk.front.yield.basisdelta.isparallel"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_SHIFTTYPE  ".risk.front.yield.basisdelta.shifttype"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BASISTYPE  ".risk.front.yield.basisdelta.basistype"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_PARALLEL_SHIFTVAL  ".risk.front.yield.basisdelta.parallel.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_TERM  ".risk.front.yield.basisdelta.grid.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUCKET_GRID_TERM ".risk.front.yield.basisdelta.bucket.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_SHIFTVAL  ".risk.front.yield.basisdelta.grid.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_GRID_ISTERMCHECK  ".risk.front.yield.basisdelta.grid.istermcheck"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_BUMPDIRECTION  ".risk.front.yield.basisdelta.bumpdirection"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISWAVE  ".risk.front.yield.basisdelta.iswave"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_DIVUNIT  ".risk.front.yield.basisdelta.divunit"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISRISKCURRENCYMODE ".risk.front.yield.basisdelta.isriskcurrencymode"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISZERORATEBUMP ".risk.front.yield.basisdelta.iszeroratebump"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_OMITNOTIONALEXPOSURE ".risk.front.yield.basisdelta.omitnotionalexposure"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISIMMFWDRATEBUMP ".risk.front.yield.basisdelta.isimmfwdratebump"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_IMMFWDRATETERM ".risk.front.yield.basisdelta.immfwdrateterm"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_RECALCBASISDFSONZERORATEBUMP ".risk.front.yield.basisdelta.recalcbasisdfsonzeroratebump"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISFWDFXZERORATEBUMP  ".risk.front.yield.basisdelta.isfwdfxzeroratebump"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_BASISDELTA_ISFWDPOINTBUMP  ".risk.front.yield.basisdelta.isfwdpointbump"

// FX DELTA
#define RISK_FRONT_FX_DELTA_TARGET_FX  "risk.front.fx.delta.target.fx"
#define RISK_FRONT_FX_DELTA_CALIBRATION_TARGET_FX  "risk.front.fx.delta.calibration.target.fx"
#define RISK_FRONT_FX_DELTA_ISCALCGAMMA "risk.front.fx.delta.iscalcgamma"
#define FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME  ".risk.front.fx.delta.output"
#define FX_KEY_RISK_FRONT_FX_DELTA_OUTPUTNAME2  ".risk.front.fx.delta.output2"
#define FX_KEY_RISK_FRONT_FX_DELTA_SHIFTTYPE  ".risk.front.fx.delta.shifttype"
#define FX_KEY_RISK_FRONT_FX_DELTA_SPOT_SHIFTVAL  ".risk.front.fx.delta.spot.shiftval"
#define FX_KEY_RISK_FRONT_FX_DELTA_BUMPDIRECTION  ".risk.front.fx.delta.bumpdirection"
#define FX_KEY_RISK_FRONT_FX_DELTA_DIVUNIT  ".risk.front.fx.delta.divunit"
#define FX_KEY_RISK_FRONT_FX_DELTA_ISRISKCURRENCYMODE ".risk.front.fx.delta.isriskcurrencymode"
#define FX_KEY_RISK_FRONT_FX_DELTA_ISCOEFFICIENTRATIO ".risk.front.fx.delta.iscoefficientratio"
#define FX_KEY_RISK_FRONT_FX_DELTA_OMITNOTIONALEXPOSURE ".risk.front.fx.delta.omitnotionalexposure"

// FX VEGA
#define RISK_FRONT_VOL_FXVEGA_TARGET_FX  "risk.front.vol.fxvega.target.fx"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_OUTPUTNAME  ".risk.front.vol.fxvega.output"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SHIFTTYPE  ".risk.front.vol.fxvega.shifttype"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_ISGRIDSENSITIVITY  ".risk.front.vol.fxvega.isgridsensitivity"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_ISPARALLEL  ".risk.front.vol.fxvega.isparallel"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_DELTATYPE  ".risk.front.vol.fxvega.deltatype"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_GRID  ".risk.front.vol.fxvega.grid"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_BUCKET_GRID_TERM ".risk.front.vol.fxvega.bucket.term"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_BUMPDIRECTION  ".risk.front.vol.fxvega.bumpdirection"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_SHIFTVAL  ".risk.front.vol.fxvega.shiftval"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_DIVUNIT  ".risk.front.vol.fxvega.divunit"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_ISWAVE  ".risk.front.vol.fxvega.iswave"
#define FX_KEY_RISK_FRONT_VOL_FXVEGA_ISRISKCURRENCYMODE ".risk.front.vol.fxvega.isriskcurrencymode"


//FX SHIFTDELTA
#define STATIC_DATA_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SHIFTVAL  ".risk.front.fx.fxshiftdelta.shiftval"
#define RISK_FRONT_FX_FXSHIFTDELTA_TARGET_FX  "risk.front.fx.fxshiftdelta.target.fx"
#define RISK_FRONT_FX_FXSHIFTDELTA_CALIBRATION_TARGET_FX  "risk.front.fx.fxshiftdelta.calibration.target.fx"
#define FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_OUTPUTNAME  ".risk.front.fx.fxshiftdelta.output"
#define FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SHIFTTYPE  ".risk.front.fx.fxshiftdelta.shifttype"
#define FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_SPOT_SHIFTVAL  ".risk.front.fx.fxshiftdelta.spot.shiftval"
#define FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_BUMPDIRECTION  ".risk.front.fx.fxshiftdelta.bumpdirection"
#define FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_DIVUNIT  ".risk.front.fx.fxshiftdelta.divunit"
#define FX_KEY_RISK_FRONT_FX_FXSHIFTDELTA_ISRISKCURRENCYMODE ".risk.front.fx.fxshiftdelta.isriskcurrencymode"

//FXSHIFT FX VEGA
#define RISK_FRONT_VOL_FXSHIFTFXVEGA_TARGET_FX  "risk.front.vol.fxshiftfxvega.target.fx"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_BASESHIFTVAL  ".risk.front.vol.fxshiftfxvega.baseshiftval"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_SHIFTVAL  ".risk.front.vol.fxshiftfxvega.shiftval"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_OUTPUTNAME  ".risk.front.vol.fxshiftfxvega.output"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_SHIFTTYPE  ".risk.front.vol.fxshiftfxvega.shifttype"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISGRIDSENSITIVITY  ".risk.front.vol.fxshiftfxvega.isgridsensitivity"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISPARALLEL  ".risk.front.vol.fxshiftfxvega.isparallel"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_DELTATYPE  ".risk.front.vol.fxshiftfxvega.deltatype"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_GRID  ".risk.front.vol.fxshiftfxvega.grid"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_BUCKET_GRID_TERM ".risk.front.vol.fxshiftfxvega.bucket.term"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_BUMPDIRECTION  ".risk.front.vol.fxshiftfxvega.bumpdirection"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_DIVUNIT  ".risk.front.vol.fxshiftfxvega.divunit"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISWAVE  ".risk.front.vol.fxshiftfxvega.iswave"
#define FX_KEY_RISK_FRONT_VOL_FXSHIFTFXVEGA_ISRISKCURRENCYMODE ".risk.front.vol.fxshiftfxvega.isriskcurrencymode"


//FXSHIFT IR DELTA
#define RISK_FRONT_YIELD_FXSHIFTIRDELTA_TARGET_CURRENCY  "risk.front.yield.fxshiftirdelta.target.currency"
#define RISK_FRONT_YIELD_FXSHIFTIRDELTA_CALIBRATION_TARGET_CURRENCY  "risk.front.yield.fxshiftirdelta.calibration.target.currency"
#define RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_CALCBUFFER  "risk.front.yield.fxshiftirdelta.grid.calcbuffer"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_OUTPUTNAME  ".risk.front.yield.fxshiftirdelta.output"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISGRIDSENSITIVITY  ".risk.front.yield.fxshiftirdelta.isgridsensitivity"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISPARALLEL  ".risk.front.yield.fxshiftirdelta.isparallel"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_SHIFTTYPE  ".risk.front.yield.fxshiftirdelta.shifttype"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_PARALLEL_SHIFTVAL  ".risk.front.yield.fxshiftirdelta.parallel.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_TERM  ".risk.front.yield.fxshiftirdelta.grid.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUCKET_TERM  ".risk.front.yield.fxshiftirdelta.bucket.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_GRID_SHIFTVAL  ".risk.front.yield.fxshiftirdelta.grid.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BUMPDIRECTION  ".risk.front.yield.fxshiftirdelta.bumpdirection"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISWAVE  ".risk.front.yield.fxshiftirdelta.iswave"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_DIVUNIT  ".risk.front.yield.fxshiftirdelta.divunit"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_BASESHIFTVAL  ".risk.front.yield.fxshiftirdelta.baseshiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_CROSSBASECURRENCY  ".risk.front.yield.fxshiftirdelta.crossbasecurrency"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISRISKCURRENCYMODE ".risk.front.yield.fxshiftirdelta.isriskcurrencymode"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTIRDELTA_ISZERORATEBUMP ".risk.front.yield.fxshiftirdelta.iszeroratebump"

//FXSHIFT BASIS DELTA
#define RISK_FRONT_YIELD_FXSHIFTBASISDELTA_TARGET_CURRENCY  "risk.front.yield.fxshiftbasisdelta.target.currency"
#define RISK_FRONT_YIELD_FXSHIFTBASISDELTA_CALIBRATION_TARGET_CURRENCY  "risk.front.yield.fxshiftbasisdelta.calibration.target.currency"
#define RISK_FRONT_YIELD_FXSHIFTBASISDELTA_GRID_CALCBUFFER  "risk.front.yield.fxshiftbasisdelta.grid.calcbuffer"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_OUTPUTNAME  ".risk.front.yield.fxshiftbasisdelta.output"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISGRIDSENSITIVITY  ".risk.front.yield.fxshiftbasisdelta.isgridsensitivity"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISPARALLEL  ".risk.front.yield.fxshiftbasisdelta.isparallel"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_SHIFTTYPE  ".risk.front.yield.fxshiftbasisdelta.shifttype"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BASISTYPE  ".risk.front.yield.fxshiftbasisdelta.basistype"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_PARALLEL_SHIFTVAL  ".risk.front.yield.fxshiftbasisdelta.parallel.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_GRID_TERM  ".risk.front.yield.fxshiftbasisdelta.grid.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BUCKET_TERM  ".risk.front.yield.fxshiftbasisdelta.bucket.term"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_GRID_SHIFTVAL  ".risk.front.yield.fxshiftbasisdelta.grid.shiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BUMPDIRECTION  ".risk.front.yield.fxshiftbasisdelta.bumpdirection"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISWAVE  ".risk.front.yield.fxshiftbasisdelta.iswave"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_DIVUNIT  ".risk.front.yield.fxshiftbasisdelta.divunit"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_BASESHIFTVAL  ".risk.front.yield.fxshiftbasisdelta.baseshiftval"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_CROSSBASECURRENCY  ".risk.front.yield.fxshiftbasisdelta.crossbasecurrency"
#define STATIC_DATA_KEY_RISK_FRONT_YIELD_FXSHIFTBASISDELTA_ISRISKCURRENCYMODE ".risk.front.yield.fxshiftbasisdelta.isriskcurrencymode"

// FXVOLSHIFT FX VEGA
#define RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_TARGET_FX  "risk.front.vol.fxvolshiftfxvega.target.fx"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_OUTPUTNAME  ".risk.front.vol.fxvolshiftfxvega.output"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_SHIFTTYPE  ".risk.front.vol.fxvolshiftfxvega.shifttype"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISGRIDSENSITIVITY  ".risk.front.vol.fxvolshiftfxvega.isgridsensitivity"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISPARALLEL  ".risk.front.vol.fxvolshiftfxvega.isparallel"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_DELTATYPE  ".risk.front.vol.fxvolshiftfxvega.deltatype"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_GRID  ".risk.front.vol.fxvolshiftfxvega.grid"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUCKET_GRID_TERM ".risk.front.vol.fxvolshiftfxvega.bucket.term"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BUMPDIRECTION  ".risk.front.vol.fxvolshiftfxvega.bumpdirection"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_SHIFTVAL  ".risk.front.vol.fxvolshiftfxvega.shiftval"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_DIVUNIT  ".risk.front.vol.fxvolshiftfxvega.divunit"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISWAVE  ".risk.front.vol.fxvolshiftfxvega.iswave"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_BASESHIFTVOLVAL  ".risk.front.vol.fxvolshiftfxvega.baseshiftvolval"
#define FX_KEY_RISK_FRONT_VOL_FXVOLSHIFTFXVEGA_ISRISKCURRENCYMODE ".risk.front.vol.fxvolshiftfxvega.isriskcurrencymode"

//IRSHIFT FX DELTA
#define RISK_FRONT_FX_IRSHIFTFXDELTA_TARGET_FX  "risk.front.fx.irshiftfxdelta.target.fx"
#define RISK_FRONT_FX_IRSHIFTFXDELTA_CALIBRATION_TARGET_FX  "risk.front.fx.irshiftfxdelta.calibration.target.fx"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_SPOT_SHIFTVAL  ".risk.front.fx.irshiftfxdelta.spot.shiftval"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_OUTPUTNAME  ".risk.front.fx.irshiftfxdelta.output"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_BUMPDIRECTION  ".risk.front.fx.irshiftfxdelta.bumpdirection"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_SHIFTTYPE  ".risk.front.fx.irshiftfxdelta.shifttype"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_DIVUNIT  ".risk.front.fx.irshiftfxdelta.divunit"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_CROSSBASECURRENCY	".risk.front.fx.irshiftfxdelta.crossbasecurrency"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_GRID_TERM  ".risk.front.fx.irshiftfxdelta.grid.term"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_BASESHIFTVAL  ".risk.front.fx.irshiftfxdelta.baseshiftval"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXDELTA_ISRISKCURRENCYMODE ".risk.front.fx.irshiftfxdelta.isriskcurrencymode"

//IRSHIFT FX VEGA
#define RISK_FRONT_VOL_IRSHIFTFXVEGA_TARGET_FX  "risk.front.vol.irshiftfxvega.target.fx"
#define RISK_FRONT_VOL_IRSHIFTFXVEGA_CALIBRATION_TARGET_FX  "risk.front.vol.irshiftfxvega.calibration.target.fx"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_OUTPUTNAME  ".risk.front.vol.irshiftfxvega.output"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_SHIFTTYPE  ".risk.front.vol.irshiftfxvega.shifttype"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISGRIDSENSITIVITY  ".risk.front.vol.irshiftfxvega.isgridsensitivity"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISPARALLEL  ".risk.front.vol.irshiftfxvega.isparallel"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_DELTATYPE  ".risk.front.vol.irshiftfxvega.deltatype"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_GRID  ".risk.front.vol.irshiftfxvega.grid"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUCKET_GRID_TERM ".risk.front.vol.irshiftfxvega.bucket.term"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BUMPDIRECTION  ".risk.front.vol.irshiftfxvega.bumpdirection"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_SHIFTVAL  ".risk.front.vol.irshiftfxvega.shiftval"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_DIVUNIT  ".risk.front.vol.irshiftfxvega.divunit"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISWAVE  ".risk.front.vol.irshiftfxvega.iswave"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_CROSSBASECURRENCY ".risk.front.vol.irshiftfxvega.crossbasecurrency"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_BASESHIFTVAL  ".risk.front.vol.irshiftfxvega.baseshiftval"
#define FX_KEY_RISK_FRONT_FX_IRSHIFTFXVEGA_GRID_TERM  ".risk.front.vol.irshiftfxvega.grid.term"
#define FX_KEY_RISK_FRONT_VOL_IRSHIFTFXVEGA_ISRISKCURRENCYMODE ".risk.front.vol.irshiftfxvega.isriskcurrencymode"

//COR DELTA
#define RISK_FRONT_COR_CORDELTA_TARGET "risk.front.cor.cordelta.target"
#define COR_KEY_RISK_FRONT_COR_CORDELTA_BUMPDIRECTION ".risk.front.cor.cordelta.bumpdirection"
#define COR_KEY_RISK_FRONT_COR_CORDELTA_OUTPUTNAME ".risk.front.cor.cordelta.output"
#define COR_KEY_RISK_FRONT_COR_CORDELTA_DIVUNIT ".risk.front.cor.cordelta.divunit"
#define COR_KEY_RISK_FRONT_COR_CORDELTA_SHIFTVAL ".risk.front.cor.cordelta.shiftval"
#define COR_KEY_RISK_FRONT_COR_CORDELTA_ISRISKCURRENCYMODE ".risk.front.cor.cordelta.isriskcurrencymode"

//THETA
#define RISK_OFFICIAL_THETA_SHIFTVAL  "risk.official.theta.shiftval"
#define RISK_OFFICIAL_THETA_DIVUNIT  "risk.official.theta.divunit"
#define RISK_OFFICIAL_THETA_OUTPUTNAME  "risk.official.theta.outputname"
#define RISK_OFFICIAL_THETA_ISRISKCURRENCYMODE  "risk.official.theta.isriskcurrencymode"
#define RISK_OFFICIAL_THETA_WITHOUTCASH  "risk.official.theta.withoutcash"
#define RISK_OFFICIAL_THETA_CALENDAR  "risk.official.theta.calendar"

//CREDITSHIFT
#define ENTITY_KEY_RISK_FRONT_HAZARD_CREDITSPREADDELTA_OUTPUTNAME  ".risk.front.hazard.creditspreaddelta.outputname"
#define ENTITY_KEY_RISK_FRONT_HAZARD_CREDITSPREADDELTA_SHIFTTYPE  ".risk.front.hazard.creditspreaddelta.shifttype"
#define ENTITY_KEY_RISK_FRONT_HAZARD_CREDITSPREADDELTA_SHIFTVAL  ".risk.front.hazard.creditspreaddelta.shiftval"
#define ENTITY_KEY_RISK_FRONT_HAZARD_CREDITSPREADDELTA_DIVUNIT  ".risk.front.hazard.creditspreaddelta.divunit"

// EQDELTA
#define RISK_EQ_DELTA_OUTPUTNAME "risk.eq.delta.output"
#define RISK_EQ_DELTA_SHIFTTYPE "risk.eq.delta.shifttype"
#define RISK_EQ_DELTA_SPOT_SHIFTVAL "risk.eq.delta.spot.shiftval"
#define RISK_EQ_DELTA_BUMPDIRECTION "risk.eq.delta.bumpdirection"
#define RISK_EQ_DELTA_DIVUNIT "risk.eq.delta.divunit"
#define RISK_EQ_DELTA_ISSCENARIOIIJIMA "risk.eq.delta.isscenarioiijima"

// EQGAMMA
#define RISK_EQ_GAMMA_OUTPUTNAME "risk.eq.gamma.output"
#define RISK_EQ_GAMMA_SHIFTTYPE "risk.eq.gamma.shifttype"
#define RISK_EQ_GAMMA_SPOT_SHIFTVAL "risk.eq.gamma.spot.shiftval"
#define RISK_EQ_GAMMA_DIVUNIT "risk.eq.gamma.divunit"
#define RISK_EQ_GAMMA_ISSCENARIOIIJIMA "risk.eq.gamma.isscenarioiijima"

// EQATMVEGA
#define RISK_EQ_ATM_VEGA_OUTPUTNAME "risk.eq.atm.vega.output"
#define RISK_EQ_ATM_VEGA_SHIFTTYPE "risk.eq.atm.vega.shifttype"
#define RISK_EQ_ATM_VEGA_ISGRIDSENSITIVITY "risk.eq.atm.vega.isgridsensitivity"
#define RISK_EQ_ATM_VEGA_ISPARALLEL "risk.eq.atm.vega.isparallel"
#define RISK_EQ_ATM_VEGA_BUMPDIRECTION "risk.eq.atm.vega.bumpdirection"
#define RISK_EQ_ATM_VEGA_SHIFTVAL "risk.eq.atm.vega.shiftval"
#define RISK_EQ_ATM_VEGA_DIVUNIT "risk.eq.atm.vega.divunit"
#define RISK_EQ_ATM_VEGA_ISWAVE "risk.eq.atm.vega.iswave"
#define RISK_EQ_ATM_VEGA_ISBACKWARD "risk.eq.atm.vega.isbackward"
#define RISK_EQ_ATM_VEGA_ISBUMPLIMIT "risk.eq.atm.vega.isbumplimit"

// EQSKEWVEGA
#define RISK_EQ_SKEW_VEGA_OUTPUTNAME "risk.eq.skew.vega.output"
#define RISK_EQ_SKEW_VEGA_SHIFTTYPE "risk.eq.skew.vega.shifttype"
#define RISK_EQ_SKEW_VEGA_ISGRIDSENSITIVITY "risk.eq.skew.vega.isgridsensitivity"
#define RISK_EQ_SKEW_VEGA_ISPARALLEL "risk.eq.skew.vega.isparallel"
#define RISK_EQ_SKEW_VEGA_BUMPDIRECTION "risk.eq.skew.vega.bumpdirection"
#define RISK_EQ_SKEW_VEGA_SHIFTVAL "risk.eq.skew.vega.shiftval"
#define RISK_EQ_SKEW_VEGA_DIVUNIT "risk.eq.skew.vega.divunit"
#define RISK_EQ_SKEW_VEGA_ISWAVE "risk.eq.skew.vega.iswave"
#define RISK_EQ_SKEW_VEGA_ISBACKWARD "risk.eq.skew.vega.isbackward"

// EQKURTVEGA
#define RISK_EQ_KURT_VEGA_OUTPUTNAME "risk.eq.kurt.vega.output"
#define RISK_EQ_KURT_VEGA_SHIFTTYPE "risk.eq.kurt.vega.shifttype"
#define RISK_EQ_KURT_VEGA_ISGRIDSENSITIVITY "risk.eq.kurt.vega.isgridsensitivity"
#define RISK_EQ_KURT_VEGA_ISPARALLEL "risk.eq.kurt.vega.isparallel"
#define RISK_EQ_KURT_VEGA_BUMPDIRECTION "risk.eq.kurt.vega.bumpdirection"
#define RISK_EQ_KURT_VEGA_SHIFTVAL "risk.eq.kurt.vega.shiftval"
#define RISK_EQ_KURT_VEGA_DIVUNIT "risk.eq.kurt.vega.divunit"
#define RISK_EQ_KURT_VEGA_ISWAVE "risk.eq.kurt.vega.iswave"
#define RISK_EQ_KURT_VEGA_ISBACKWARD "risk.eq.kurt.vega.isbackward"

// EQREPORATEDELTA
#define RISK_EQ_REPORATE_DELTA_OUTPUTNAME "risk.eq.reporate.delta.output"
#define RISK_EQ_REPORATE_DELTA_SHIFTTYPE "risk.eq.reporate.delta.shifttype"
#define RISK_EQ_REPORATE_DELTA_ISGRIDSENSITIVITY "risk.eq.reporate.delta.isgridsensitivity"
#define RISK_EQ_REPORATE_DELTA_ISPARALLEL "risk.eq.reporate.delta.isparallel"
#define RISK_EQ_REPORATE_DELTA_BUMPDIRECTION "risk.eq.reporate.delta.bumpdirection"
#define RISK_EQ_REPORATE_DELTA_SHIFTVAL "risk.eq.reporate.delta.shiftval"
#define RISK_EQ_REPORATE_DELTA_DIVUNIT "risk.eq.reporate.delta.divunit"
#define RISK_EQ_REPORATE_DELTA_ISWAVE "risk.eq.reporate.delta.iswave"

// EQDIVIDENDDELTA
#define RISK_EQ_DIVIDEND_DELTA_OUTPUTNAME "risk.eq.dividend.delta.output"
#define RISK_EQ_DIVIDEND_DELTA_SHIFTTYPE "risk.eq.dividend.delta.shifttype"
#define RISK_EQ_DIVIDEND_DELTA_ISGRIDSENSITIVITY "risk.eq.dividend.delta.isgridsensitivity"
#define RISK_EQ_DIVIDEND_DELTA_ISPARALLEL "risk.eq.dividend.delta.isparallel"
#define RISK_EQ_DIVIDEND_DELTA_BUMPDIRECTION "risk.eq.dividend.delta.bumpdirection"
#define RISK_EQ_DIVIDEND_DELTA_SHIFTVAL "risk.eq.dividend.delta.shiftval"
#define RISK_EQ_DIVIDEND_DELTA_DIVUNIT "risk.eq.dividend.delta.divunit"
#define RISK_EQ_DIVIDEND_DELTA_ISWAVE "risk.eq.dividend.delta.iswave"

//RISK_OFFICIAL_



//********************************
//
// property key end
//
//********************************



#endif