#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include <algorithm>
#include "AQLObject.h"
#include "AQLCoreAutoPtr.h"
#include "AQLPriceDataConvention.h"
#include "AQLFunctionBase.h"
#include "AQLFunctionVector.h"
#include "AQLMathYieldCurve.h"
#include "AQLDataBasics.h"
#include "AQLMathDateCalculations.h"
#include "AQLPriceYieldGenerator.h"

#include "AQLModelDynamicsCurve.h"
 
//// DEFINES ////
#define ENTITY_IRYCPRO  19

#ifndef IR_CALIBRATION_DATA_GENERATEMETHOD
#define IR_CALIBRATION_DATA_GENERATEMETHOD   "GenerateMethod"	   //  data name of Compound Or Not
#endif
#ifndef IR_CALIBRATION_DATA_DATATYPE
#define IR_CALIBRATION_DATA_DATATYPE			"DataType"				//  data name of DataType
#endif
#ifndef IR_CALIBRATION_DATA_INSTRUMENTSUBTYPE
#define IR_CALIBRATION_DATA_INSTRUMENTSUBTYPE		"InstrumentSubType"				//  data name of InstrumentSubType
#endif
#ifndef IR_CALIBRATION_DATA_TERM
#define IR_CALIBRATION_DATA_TERM				"Term"					//  data name of Term
#endif
#ifndef IR_CALIBRATION_DATA_SPOTDATE
#define IR_CALIBRATION_DATA_SPOTDATE			"SpotDate"				//  data name of SpotDate
#endif
#ifndef IR_CALIBRATION_DATA_SPOTLAG
#define IR_CALIBRATION_DATA_SPOTLAG			"SpotLag"				//  data name of SpotDate
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCYBASE
#define IR_CALIBRATION_DATA_FREQUENCYBASE		"Frequency_baseYC"		//  data name of Frequency_baseYC
#endif
#ifndef IR_CALIBRATION_DATA_CALENDARBASE
#define IR_CALIBRATION_DATA_CALENDARBASE		"Calendar_baseYC"		//  data name of Calendar_baseYC
#endif
#ifndef IR_CALIBRATION_DATA_SLIDINGRULEBASE
#define IR_CALIBRATION_DATA_SLIDINGRULEBASE	"SlidingRule_baseYC"	//  data name of SlidingRule_baseYC
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNTBASE
#define IR_CALIBRATION_DATA_DAYCOUNTBASE		"DayCount_baseYC"		//  data name of DayCount_baseYC
#endif
#ifndef IR_CALIBRATION_DATA_BASISFUNCTION
#define IR_CALIBRATION_DATA_BASISFUNCTION		"BasisFunction"			//  data name of BasisFunction
#endif
#ifndef IR_CALIBRATION_DATA_BASISRATES
#define IR_CALIBRATION_DATA_BASISRATES		"BasisRates"			// Data Name of BasisRates
#endif
#ifndef IR_CALIBRATION_DATA_BASISDATA
#define IR_CALIBRATION_DATA_BASISDATA			"BasisData"				// Data Name of BasisData
#endif
#ifndef IR_CALIBRATION_DATA_BASEYIELDCURVE
#define IR_CALIBRATION_DATA_BASEYIELDCURVE	"BaseYieldCurve"		// Data Name of BaseYieldCurve
#endif
#ifndef IR_CALIBRATION_DATA_INTERPOLATIONBS
#define IR_CALIBRATION_DATA_INTERPOLATIONBS	"BasisInterpolation"	// Data Name of BasisInterpolation
#endif
#ifndef IR_CALIBRATION_DATA_STARTDATE
#define IR_CALIBRATION_DATA_STARTDATE			"StartDate"				//  data name of StartDate
#endif
#ifndef IR_CALIBRATION_DATA_ISSAMEGRIDINDEX
#define IR_CALIBRATION_DATA_ISSAMEGRIDINDEX	 "IsSameGridIndex"		//  data name of IsSameGridIndex
#endif
#ifndef IR_CALIBRATION_DATA_ISODDTERMFRNINDEX
#define IR_CALIBRATION_DATA_ISODDTERMFRNINDEX	 "IsOddTermFRNIndex"	//  data name of IsOddTermFRNIndex
#endif
#ifndef IR_CALIBRATION_DATA_ISDISCOUNTCURVE
#define IR_CALIBRATION_DATA_ISDISCOUNTCURVE	 "IsDiscountCurve"		//  data name of IsDiscountCurve
#endif
#ifndef IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS
#define IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONBS	 "IsTimeInterpolationBasis"		//  data name of IsTimeInterpolationBasis
#endif
#ifndef IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONSW
#define IR_CALIBRATION_DATA_ISTIMEINTERPOLATIONSW	 "IsTimeInterpolationSwap"		//  data name of IsTimeInterpolationSwap
#endif
#ifndef IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW
#define IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW	 "IsNewtonRaphsonSwap"		//  data name of IsNewtonRaphsonSwap
#endif
#ifndef IR_CALIBRATION_DATA_OPTIMIZEMETHOD
#define IR_CALIBRATION_DATA_OPTIMIZEMETHOD	 "OptimizeMethod"		//  data name of OptimizeMethod
#endif
#ifndef IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW
#define IR_CALIBRATION_DATA_ISSIMULTANEOUSEQSW	 "IsSimultaneousEQSwap"		//  data name of IsSimultaneousEQSwap
#endif
#ifndef IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS
#define IR_CALIBRATION_DATA_ISSIMULTANEOUSEQBS	 "IsSimultaneousEQBasis"		//  data name of IsSimultaneousEQBasis
#endif
#ifndef IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE
#define IR_CALIBRATION_DATA_OPTIMIZEPERFORMANCE	 "OptimizePerformance"		//  data name of OptimizePerformance
#endif
#ifndef IR_CALIBRATION_DATA_FASTREBUILD
#define IR_CALIBRATION_DATA_FASTREBUILD	 "FastRebuild"		//  Controls whether to perform a fast jacobian rebuild
#endif
//#ifndef IR_CALIBRATION_DATA_FIXINGSOURCE
//#define IR_CALIBRATION_DATA_FIXINGSOURCE			"FixingSource"		//  data name of FixingSource
//#endif
#ifndef IR_CALIBRATION_DATA_ISCONVADJPRECISE
#define IR_CALIBRATION_DATA_ISCONVADJPRECISE	 "IsConvAdjPrecise"		//  data name of IsConvAdjPrecise
#endif
#ifndef IR_CALIBRATION_DATA_MEANREVERSION
#define IR_CALIBRATION_DATA_MEANREVERSION	 "MeanReversion"		//  data name of MeanReversion
#endif
#ifndef IR_CALIBRATION_DATA_APPLYTENSION
#define IR_CALIBRATION_DATA_APPLYTENSION	 "ApplyTension" //  data name of ApplyTension
#endif
#ifndef IR_CALIBRATION_DATA_TENSIONGAP
#define IR_CALIBRATION_DATA_TENSIONGAP	 "TensionGap" //  data name of TensionGap
#endif
#ifndef IR_CALIBRATION_DATA_ISFWDBASIS
#define IR_CALIBRATION_DATA_ISFWDBASIS	 "IsFwdBasis"		//  data name of IsFwdSwap
#endif
#ifndef IR_CALIBRATION_DATA_ISDATE
#define IR_CALIBRATION_DATA_ISDATE	 "IsDate"		//  data name of IsDate
#endif
#ifndef IR_CALIBRATION_DATA_STARTTERM
#define IR_CALIBRATION_DATA_STARTTERM		"StartTerm"				//  data name of Start Term
#endif
#ifndef IR_CALIBRATION_DATA_TENOR
#define IR_CALIBRATION_DATA_TENOR		"Tenor"				//  data name of Tenor
#endif
#ifndef IR_CALIBRATION_DATA_ISEOMROLLSW
#define IR_CALIBRATION_DATA_ISEOMROLLSW	 "IsEOMRollSwap"		//  data name of IsEOMRollSwap
#endif
#ifndef IR_CALIBRATION_DATA_ISEOMROLLOIS
#define IR_CALIBRATION_DATA_ISEOMROLLOIS	 "IsEOMRollOIS"		//  data name of IsEOMRollOIS
#endif
#ifndef IR_CALIBRATION_DATA_ISEOMROLL
#define IR_CALIBRATION_DATA_ISEOMROLL	 "IsEOMRoll"		//  data name of IsEOMRoll(Other)
#endif
#ifndef IR_CALIBRATION_DATA_ISONFORSPOTADJUST
#define IR_CALIBRATION_DATA_ISONFORSPOTADJUST	 "IsONForSpotAdjust"		//  data name of IsONForSpotAdjust
#endif
#ifndef IR_CALIBRATION_DATA_ISSWAPTENORADJUST
#define IR_CALIBRATION_DATA_ISSWAPTENORADJUST	 "IsSwapTenorAdjust"		//  data name of IsSwapTenorAdjust
#endif
#ifndef IR_CALIBRATION_DATA_USETENORBASIS
#define IR_CALIBRATION_DATA_USETENORBASIS	 "UseTenorBasis"		//  data name of AllowMixedSwaps <--- An alias for IsSwapTenorAdjust
#endif
#ifndef IR_CALIBRATION_DATA_FOREIGNYIELDDATA
#define IR_CALIBRATION_DATA_FOREIGNYIELDDATA	 "ForeignYieldData"		//  data name of ForeignYieldData
#endif
#ifndef IR_CALIBRATION_DATA_TENORSWAPNAME
#define IR_CALIBRATION_DATA_TENORSWAPNAME	 "TenorSwapName"		//  data name of TenorSwapName
#endif
#ifndef IR_CALIBRATION_DATA_GENERATEDFS
#define IR_CALIBRATION_DATA_GENERATEDFS	 "GenerateDFS"		//  data name of GenerateDFS
#endif
#ifndef IR_CALIBRATION_DATA_GENTARGETDF
#define IR_CALIBRATION_DATA_GENTARGETDF	 "GenTargetDF"		//  data name of GenerateDFS
#endif
#ifndef IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE
#define IR_CALIBRATION_DATA_DUALBOOTSTRAP_SWAPCURVE	"DualBootstrapSwapCurve"		// data name of DualBootstrapSwapCurve
#endif
#ifndef IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE
#define IR_CALIBRATION_DATA_DUALBOOTSTRAP_OISCURVE	"DualBootstrapOISCurve"		// data name of DualBootstrapOISCurve
#endif

#ifndef IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON
#define IR_CALIBRATION_DATA_GLOBALENGINECURVES_EPSILON				"GlobalEngineCurvesEpsilon"		//  data name of GlobalEngineCurvesEpsilon
#endif
#ifndef IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON
#define IR_CALIBRATION_DATA_GLOBALENGINECURVES_GRADIENTEPSILON		"GlobalEngineCurvesGradientEpsilon"		//  data name of GlobalEngineCurvesGradientEpsilon
#endif
#ifndef IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA
#define IR_CALIBRATION_DATA_GLOBALENGINECURVES_DELTA				"GlobalEngineCurvesDelta"		//  data name of GlobalEngineCurvesDelta
#endif
#ifndef IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP
#define IR_CALIBRATION_DATA_GLOBALENGINECURVES_MAXLOOP				"GlobalEngineCurvesMaxLoop"		//  data name of GlobalEngineCurvesMaxLoop
#endif
#ifndef IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD
#define IR_CALIBRATION_DATA_GLOBALENGINECURVES_FASTREBUILD			"GlobalEngineCurvesFastRebuild"		// data name of GlobalEngineCurvesFastRebuild
#endif

#ifndef IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES
#define IR_CALIBRATION_DATA_GLOBALENGINECURVES_ALLCURVENAMES	"GlobalEngineCurvesAllCurveNames"		// data name of GlobalEngineCurvesAllCurveNames
#endif

#ifndef IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE
#define IR_CALIBRATION_DATA_GLOBALENGINECURVES_SINGLECURVETYPE	"GlobalEngineCurvesSingleCurveType"		// data name of GlobalEngineCurvesSingleCurveType
#endif

#ifndef IR_CALIBRATION_DATA_IS2SWAP
#define IR_CALIBRATION_DATA_IS2SWAP	 "Is2Swap"		//  data name of Is2Swap
#endif

#ifndef IR_CALIBRATION_DATA_BASISTARGETDF
#define IR_CALIBRATION_DATA_BASISTARGETDF	 "BasisTargetDF"		//  data name of BasisTargetDF
#endif
#ifndef IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS
#define IR_CALIBRATION_CALIBRATION_DATAOGENERATEDFS	 "NoGenerateDFS"		//  data name of NoGenerateDFS
#endif
#ifndef IR_CALIBRATION_DATA_BASISDFS
#define IR_CALIBRATION_DATA_BASISDFS	 "BasisDFS"		//  data name of BasisDFs
#endif
#ifndef IR_CALIBRATION_DATA_FLOATERDFS
#define IR_CALIBRATION_DATA_FLOATERDFS	 "FloaterDFS"		//  data name of FloaterDFS
#endif
#ifndef IR_CALIBRATION_DATA_MAINBASISDF
#define IR_CALIBRATION_DATA_MAINBASISDF	 "MainBasisDF"		//  data name of MainBasisDF
#endif
#ifndef IR_CALIBRATION_DATA_FORECAST
#define IR_CALIBRATION_DATA_FORECAST	 "Forecast"		//  data name of Forecast
#endif
#ifndef IR_CALIBRATION_DATA_DISCOUNT
#define IR_CALIBRATION_DATA_DISCOUNT	 "Discount"		//  data name of Discount
#endif
#ifndef IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR
#define IR_CALIBRATION_DATA_INDEXFIXINGCALENDAR	 "IndexFixingCalendar"		//  data name of IndexFixingCalendar
#endif
#ifndef IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR
#define IR_CALIBRATION_DATA_INDEXPAYMENTCALENDAR	 "IndexPaymentCalendar"		//  data name of IndexPaymentCalendar
#endif
#ifndef IR_CALIBRATION_DATA_INDEXFREQUENCY
#define IR_CALIBRATION_DATA_INDEXFREQUENCY	 "IndexFrequency"		//  data name of IndexFrequency
#endif
#ifndef IR_CALIBRATION_DATA_INDEXSLIDINGRULE
#define IR_CALIBRATION_DATA_INDEXSLIDINGRULE	 "IndexSlidingRule"		//  data name of IndexSlidingRule
#endif
#ifndef IR_CALIBRATION_DATA_INDEXDAYCOUNT
#define IR_CALIBRATION_DATA_INDEXDAYCOUNT	 "IndexDaycount"		//  data name of IndexDaycount
#endif
#ifndef IR_CALIBRATION_DATA_INDEXACCESSARY
#define IR_CALIBRATION_DATA_INDEXACCESSARY	 "IndexAccessary"		//  data name of IndexAccessary
#endif
#ifndef IR_CALIBRATION_DATA_INDEXRESETLAG
#define IR_CALIBRATION_DATA_INDEXRESETLAG	 "IndexResetLag"		//  data name of IndexSpotDate
#endif

#ifndef IR_CALIBRATION_DATA_AGTCURRENCY
#define IR_CALIBRATION_DATA_AGTCURRENCY	 "AgtCurrency"		//  data name of AgtCurrency
#endif
#ifndef IR_CALIBRATION_DATA_AGTFORECAST
#define IR_CALIBRATION_DATA_AGTFORECAST	 "AgtForecast"		//  data name of AgtForecast
#endif
#ifndef IR_CALIBRATION_DATA_AGTDISCOUNT
#define IR_CALIBRATION_DATA_AGTDISCOUNT	 "AgtDiscount"		//  data name of AgtDiscount
#endif
#ifndef IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR
#define IR_CALIBRATION_DATA_AGTINDEXFIXINGCALENDAR	 "AgtIndexFixingCalendar"		//  data name of AgtIndexFixingCalendar
#endif
#ifndef IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR
#define IR_CALIBRATION_DATA_AGTINDEXPAYMENTCALENDAR	 "AgtIndexPaymentCalendar"		//  data name of AgtIndexPaymentCalendar
#endif
#ifndef IR_CALIBRATION_DATA_AGTINDEXFREQUENCY
#define IR_CALIBRATION_DATA_AGTINDEXFREQUENCY	 "AgtIndexFrequency"		//  data name of AgtIndexFrequency
#endif
#ifndef IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE
#define IR_CALIBRATION_DATA_AGTINDEXSLIDINGRULE	 "AgtIndexSlidingRule"		//  data name of AgtIndexSlidingRule
#endif
#ifndef IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT
#define IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT	 "AgtIndexDaycount"		//  data name of AgtIndexDaycount
#endif
#ifndef IR_CALIBRATION_DATA_AGTINDEXACCESSARY
#define IR_CALIBRATION_DATA_AGTINDEXACCESSARY	 "AgtIndexAccessary"		//  data name of AgtIndexAccessary
#endif
#ifndef IR_CALIBRATION_DATA_AGTINDEXRESETLAG
#define IR_CALIBRATION_DATA_AGTINDEXRESETLAG	 "AgtIndexResetLag"		//  data name of AgtIndexResetLag
#endif
#ifndef IR_CALIBRATION_DATA_ISDISCOUNT
#define IR_CALIBRATION_DATA_ISDISCOUNT	 "IsDiscount"		//  data name of Discount
#endif
#ifndef IR_CALIBRATION_DATA_ISFOREIGNCCYLEG
#define IR_CALIBRATION_DATA_ISFOREIGNCCYLEG	 "IsForeignCurrencyLeg"		//  data name of foreign currency flag
#endif
#ifndef IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION
#define IR_CALIBRATION_DATA_EXTERNALCURVECOLLECTION	 "ExternalCurveCollection"		//  data name of foreign currency flag
#endif
#ifndef IR_CALIBRATION_DATA_STATEVARIABLE
#define IR_CALIBRATION_DATA_STATEVARIABLE		"StateVariable"		//  data name of StateVariable
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETCALENDAR
#define IR_CALIBRATION_DATA_CASHLETCALENDAR	 "CashletCalendar"		//  data name of CashletCalendar
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETFREQUENCY
#define IR_CALIBRATION_DATA_CASHLETFREQUENCY	 "CashletFrequency"		//  data name of CashletFrequency
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND
#define IR_CALIBRATION_DATA_CASHLETFREQUENCYCOMPOUND	 "CashletFrequencyCompound"		//  data name of CashletFrequencyCompound
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETSLIDINGRULE
#define IR_CALIBRATION_DATA_CASHLETSLIDINGRULE	 "CashletSlidingRule"		//  data name of CashletFrequency
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETDAYCOUNT
#define IR_CALIBRATION_DATA_CASHLETDAYCOUNT	 "CashletDaycount"		//  data name of IndexDaycount
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETSPOTDATE
#define IR_CALIBRATION_DATA_CASHLETSPOTDATE	 "CashletSpotDate"		//  data name of CashletSpotDate
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETRESETLAG
#define IR_CALIBRATION_DATA_CASHLETRESETLAG	 "CashletResetLag"		//  data name of CashletSpotDate
#endif
#ifndef IR_CALIBRATION_DATA_CASHLETISBACKWARD
#define IR_CALIBRATION_DATA_CASHLETISBACKWARD	 "CashletIsBackward"		//  data name of CashletIsBackward
#endif

#ifndef IR_CALIBRATION_DATA_AGTCASHLETCALENDAR
#define IR_CALIBRATION_DATA_AGTCASHLETCALENDAR	 "AgtCashletCalendar"		//  data name of AgtCashletCalendar
#endif
#ifndef IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY
#define IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY	 "AgtCashletFrequency"		//  data name of AgtCashletFrequency
#endif
#ifndef IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND
#define IR_CALIBRATION_DATA_AGTCASHLETFREQUENCYCOMPOUND	 "AgtCashletFrequencyCompound"		//  data name of AgtCashletFrequencyCompound
#endif
#ifndef IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE
#define IR_CALIBRATION_DATA_AGTCASHLETSLIDINGRULE	 "AgtCashletSlidingRule"		//  data name of CashletFrequency
#endif
#ifndef IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT
#define IR_CALIBRATION_DATA_AGTCASHLETDAYCOUNT	 "AgtCashletDaycount"		//  data name of AgtIndexDaycount
#endif
#ifndef IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE
#define IR_CALIBRATION_DATA_AGTCASHLETSPOTDATE	 "AgtCashletSpotDate"		//  data name of AgtCashletSpotDate
#endif
#ifndef IR_CALIBRATION_DATA_AGTCASHLETRESETLAG
#define IR_CALIBRATION_DATA_AGTCASHLETRESETLAG	 "AgtCashletResetLag"		//  data name of CashletSpotDate
#endif
#ifndef IR_CALIBRATION_DATA_AGTCASHLETISBACKWARD
#define IR_CALIBRATION_DATA_AGTCASHLETISBACKWARD	 "AgtCashletIsBackward"		//  data name of AgtCashletIsBackward
#endif
#ifndef IR_CALIBRATION_DATA_ISAGTSPREAD
#define IR_CALIBRATION_DATA_ISAGTSPREAD	 "IsAgtSpread"		//  data name of IsAgtSpread
#endif
#ifndef IR_CALIBRATION_DATA_ISARBFREE
#define IR_CALIBRATION_DATA_ISARBFREE	 "IsArbFree"		//  data name of IsArbFree
#endif
#ifndef IR_CALIBRATION_DATA_SHORTTERMDATE
#define IR_CALIBRATION_DATA_SHORTTERMDATE	 "ShortTermDate"		//  data name of ShortTermDate
#endif
#ifndef IR_CALIBRATION_DATA_CONVERSIONMATRIX
#define IR_CALIBRATION_DATA_CONVERSIONMATRIX	 "ConversionMatrix"		//  data name of ConversionMatrix
#endif
#ifndef IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM
#define IR_CALIBRATION_DATA_CONVERSIONMATRIXTERM	 "ConversionMatrixTerm"		//  data name of ConversionMatrix
#endif
#ifndef IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES
#define IR_CALIBRATION_DATA_CONVERSIONMATRIXTERMTYPES	 "ConversionMatrixTermTypes"		//  data name of ConversionMatrixTermTypes
#endif
#ifndef IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS
#define IR_CALIBRATION_DATA_CONVERSIONOMITGRIDS	 "ConversionOmitGrids"		//  data name of ConversionOmitGrids
#endif
#ifndef IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS
#define IR_CALIBRATION_DATA_CONVERSIONRATEGRIDS	 "ConversionRateGrids"		//  data name of ConversionRateGrids
#endif
#ifndef IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES
#define IR_CALIBRATION_DATA_CONVERSIONMARKETTYPES	 "ConversionMarketTypes"		//  data name of ConversionMarketTypes
#endif
#ifndef IR_CALIBRATION_DATA_CALCDATESFORDVZERO
#define IR_CALIBRATION_DATA_CALCDATESFORDVZERO	 "CalcDatesForDVZero"		//  data name of ConversionMatrix
#endif
#ifndef IR_CALIBRATION_DATA_ISFRAUSE
#define IR_CALIBRATION_DATA_ISFRAUSE	 "IsFRAUse"		//  data name of ConversionMatrix
#endif
#ifndef IR_CALIBRATION_DATA_DFCURVENAME
#define IR_CALIBRATION_DATA_DFCURVENAME	 "DFCurveName"		//  data name of DFCurveName
#endif
#ifndef IR_CALIBRATION_DATA_GRIDUSEFLAG
#define IR_CALIBRATION_DATA_GRIDUSEFLAG	 "GridUseFlag"		//  data name of GridUseFlag
#endif
#ifndef IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE
#define IR_CALIBRATION_DATA_INPUT_INTERPOLATIONJOINDATE	 "InputInterpolationJoinDate"		//  data name of InputInterpolationJoinDate
#endif
#ifndef IR_CALIBRATION_DATA_ENABLECALCULATION
#define IR_CALIBRATION_DATA_ENABLECALCULATION	 "IsResidentMode"	// ResidentMode = Enable Calculation Flag
#endif
#ifndef IR_CALIBRATION_DATA_XCCYBASISTERM
#define IR_CALIBRATION_DATA_XCCYBASISTERM		"XCCYBasisTerm"
#endif
#ifndef IR_CALIBRATION_DATA_SPOTRATE
#define IR_CALIBRATION_DATA_SPOTRATE		"SpotRate"		//  data name of spot rate
#endif
#ifndef IR_CALIBRATION_DATA_SPOTRATETERM
#define IR_CALIBRATION_DATA_SPOTRATETERM		"SpotRateTerm"		//  data name of spot rate term
#endif
#ifndef IR_CALIBRATION_DATA_ISCURVEEXISTCHECK
#define IR_CALIBRATION_DATA_ISCURVEEXISTCHECK	"IsCurveExistCheck"		//  data name of curve exist check
#endif
#ifndef IR_CALIBRATION_DATA_ISYIELDSPREADCALC
#define IR_CALIBRATION_DATA_ISYIELDSPREADCALC	"IsYieldSpreadCalc"		//  data name of yield spread calc
#endif
#ifndef IR_CALIBRATION_DATA_ADJUSTVALUETERM
#define IR_CALIBRATION_DATA_ADJUSTVALUETERM	"AdjustValueTerm"		//  data name of adjust value term
#endif
#ifndef IR_CALIBRATION_DATA_ADJUSTVALUE
#define IR_CALIBRATION_DATA_ADJUSTVALUE	"AdjustValue"		//  data name of adjust value
#endif
#ifndef IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION
#define IR_CALIBRATION_DATA_ADJUSTVALUEINTERPOLATION	"AdjustValueInterpolation"		//  data name of adjust value interpolation
#endif
#ifndef IR_CALIBRATION_DATA_SHORTTERMCONVENTION
#define IR_CALIBRATION_DATA_SHORTTERMCONVENTION	"ShortTermConvention"		//  data name of short term convention
#endif
#ifndef IR_CALIBRATION_DATA_FIRSTRATE
#define IR_CALIBRATION_DATA_FIRSTRATE	"FirstMarket"		//  data name of first market
#endif
#ifndef IR_CALIBRATION_DATA_ISFWDINTERPOLATION
#define IR_CALIBRATION_DATA_ISFWDINTERPOLATION	"IsFWDInterpolation"		//  data name of IsFWDInterpolation
#endif
#ifndef IR_CALIBRATION_DATA_FWDINTERPOLATION
#define IR_CALIBRATION_DATA_FWDINTERPOLATION        "FWDInterpolation"    //  data name of FWDInterpolation
#endif
#ifndef IR_CALIBRATION_DATA_HISTORICALDATES
#define IR_CALIBRATION_DATA_HISTORICALDATES        "HistoricalDates"    //  data name of HistoricalDates
#endif
#ifndef IR_CALIBRATION_DATA_HISTORICALRATES
#define IR_CALIBRATION_DATA_HISTORICALRATES        "HistoricalRates"    //  data name of HistoricalRates
#endif
#ifndef IR_CALIBRATION_DATA_ISFWDFX
#define IR_CALIBRATION_DATA_ISFWDFX	 "IsFwdFX"		//  data name of IsFwdFX
#endif
#ifndef IR_CALIBRATION_DATA_ISRATIO
#define IR_CALIBRATION_DATA_ISRATIO	 "IsRatio"		//  data name of IsRatio, note: isRatio = isFXOutright (can be Fx Outright or Forward Points)
#endif
#ifndef IR_CALIBRATION_DATA_ISFXOUTRIGHT
#define IR_CALIBRATION_DATA_ISFXOUTRIGHT	 "IsFXOutright"		//  data name of IsFXOutright, note:alias for isRatio (can be Fx Outright or Forward Points)
#endif
#ifndef IR_CALIBRATION_DATA_ISPRICECCY
#define IR_CALIBRATION_DATA_ISPRICECCY	 "IsPriceCcy"		//  data name of IsPriceCcy
#endif
#ifndef IR_CALIBRATION_DATA_ISDOMESTICCURRENCY
#define IR_CALIBRATION_DATA_ISDOMESTICCURRENCY	 "IsDomesticCurrency"		//  data name of IsDomesticCurrency, alias for IsPriceCcy
#endif
#ifndef	IR_CALIBRATION_DATA_ISRENOTIONALADJUST	
#define	IR_CALIBRATION_DATA_ISRENOTIONALADJUST	"IsRenotionalAdjust"
#endif
#ifndef	IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET	
#define	IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET	"IsXccyMarkedToMarket"
#endif
#ifndef IR_CALIBRATION_DATA_FXENTITY
#define IR_CALIBRATION_DATA_FXENTITY	 "FXEntity"		//  data name of MainBasisDF
#endif
#ifndef IR_CALIBRATION_DATA_COLYIELDDATA
#define IR_CALIBRATION_DATA_COLYIELDDATA	 "ColYieldData"		//  data name of ColYieldData
#endif
#ifndef IR_CALIBRATION_DATA_MAXTERM
#define IR_CALIBRATION_DATA_MAXTERM	 "MaxTerm"		//  data name of MaxTerm
#endif
#ifndef IR_CALIBRATION_DATA_MAXTERMFREQ
#define IR_CALIBRATION_DATA_MAXTERMFREQ	 "MaxTermFrequency"		//  data name of MaxTermFrequency
#endif
#ifndef IR_CALIBRATION_DATA_ISAFFECTINGBASECURVE
#define IR_CALIBRATION_DATA_ISAFFECTINGBASECURVE	 "IsAffectingBaseCurve"		//  data name of IsAffectBaseCurve
#endif
#ifndef IR_CALIBRATION_DATA_ADDITIONALCALIBGRID
#define IR_CALIBRATION_DATA_ADDITIONALCALIBGRID	 "AddtionalCalibGrid"		//  data name of IsAffectBaseCurve
#endif

#ifndef IR_CALIBRATION_DATA_EPSILON
#define IR_CALIBRATION_DATA_EPSILON				"Epsilon"		//  data name of Epsilon
#endif
#ifndef IR_CALIBRATION_DATA_GRADIENTEPSILON
#define IR_CALIBRATION_DATA_GRADIENTEPSILON		"GradientEpsilon"		//  data name of GradientEpsilon
#endif
#ifndef IR_CALIBRATION_DATA_DELTA
#define IR_CALIBRATION_DATA_DELTA					"Delta"		//  data name of Delta
#endif
#ifndef IR_CALIBRATION_DATA_MAXLOOP
#define IR_CALIBRATION_DATA_MAXLOOP				"MaxLoop"		//  data name of MaxLoop
#endif
#ifndef IR_CALIBRATION_DATA_SWAPTENOR
#define IR_CALIBRATION_DATA_SWAPTENOR				"SwapTenor"		//  data name of SwapTenor
#endif
#ifndef IR_CALIBRATION_DATA_REFSWAPTENOR
#define IR_CALIBRATION_DATA_REFSWAPTENOR			"RefSwapTenor"		//  data name of RefSwapTenor
#endif
#ifndef IR_CALIBRATION_DATA_REFSWAPTYPE
#define IR_CALIBRATION_DATA_REFSWAPTYPE			"RefSwapType"		//  data name of RefSwapType
#endif
#ifndef IR_CALIBRATION_DATA_SWAPTYPE
#define IR_CALIBRATION_DATA_SWAPTYPE				"SwapType"		//  data name of SwapType
#endif
#ifndef IR_CALIBRATION_DATA_SMOOTHSHORTEND
#define IR_CALIBRATION_DATA_SMOOTHSHORTEND		"SmoothShortEnd"		//  data name of SmoothShortEnd
#endif
#ifndef IR_CALIBRATION_DATA_FRAFUTURE_SERIAL_CALC_TYPE
#define IR_CALIBRATION_DATA_FRAFUTURE_SERIAL_CALC_TYPE		"SerialCalcType"		//  data name of fraFutureType
#endif
#ifndef IR_CALIBRATION_DATA_FRAFUTURE_INSTRUMENT_TYPE
#define IR_CALIBRATION_DATA_FRAFUTURE_INSTRUMENT_TYPE		"InstrumentType"		//  data name of fraFutureType
#endif
#ifndef IR_CALIBRATION_DATA_MARKETTYPE
#define IR_CALIBRATION_DATA_MARKETTYPE			"MarketType"		//  data name of MainBasisDF
#endif
#ifndef IR_CALIBRATION_DATA_COMPOUNDINGMETHOD
#define IR_CALIBRATION_DATA_COMPOUNDINGMETHOD		"CompoundingMethod"		//  data name of swap compounding method
#endif
#ifndef IR_CALIBRATION_DATA_LONGTERMCONVENTION
#define IR_CALIBRATION_DATA_LONGTERMCONVENTION	"LongTermConvention"		//  data name of long term convention
#endif
#ifndef IR_CALIBRATION_DATA_LONGTERM
#define IR_CALIBRATION_DATA_LONGTERM				"LongTerm"		//  data name of long term
#endif
#ifndef IR_CALIBRATION_DATA_LONGTERMGENMETHOD
#define IR_CALIBRATION_DATA_LONGTERMGENMETHOD		"LongTermGenerateMethod"		//  data name of long term generate method
#endif
#ifndef IR_CALIBRATION_DATA_RATE_LOBASIS
#define IR_CALIBRATION_DATA_RATE_LOBASIS			"RateLOBasis"			//  data name of libor-ois basis rate
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS
#define IR_CALIBRATION_DATA_DAYCOUNT_LOBASIS		"DaycountLOBasis"		//  data name of libor-ois basis daycount
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCY_LOBASIS
#define IR_CALIBRATION_DATA_FREQUENCY_LOBASIS		"FrequencyLOBasis"		// Data Name of libor-ois basis frequency
#endif
#ifndef IR_CALIBRATION_DATA_CALENDAR_LOBASIS
#define IR_CALIBRATION_DATA_CALENDAR_LOBASIS		"CalendarLOBasis"		// Data Name of libor-ois basis calendar
#endif
#ifndef IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS
#define IR_CALIBRATION_DATA_SLIDINGRULE_LOBASIS	"SlidingRuleLOBasis"	// Data Name of libor-ois basis slidingrule
#endif
#ifndef IR_CALIBRATION_DATA_RATE_SWAP
#define IR_CALIBRATION_DATA_RATE_SWAP				"RateSwap"			//  data name of swap rate
#endif
#ifndef IR_CALIBRATION_DATA_DAYCOUNT_SWAP
#define IR_CALIBRATION_DATA_DAYCOUNT_SWAP			"DaycountSwap"		//  data name of swap daycount
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCY_SWAP
#define IR_CALIBRATION_DATA_FREQUENCY_SWAP		"FrequencySwap"		// Data Name of swap frequency
#endif
#ifndef IR_CALIBRATION_DATA_CALENDAR_SWAP
#define IR_CALIBRATION_DATA_CALENDAR_SWAP			"CalendarSwap"		// Data Name of swap calendar
#endif
#ifndef IR_CALIBRATION_DATA_SLIDINGRULE_SWAP
#define IR_CALIBRATION_DATA_SLIDINGRULE_SWAP		"SlidingRuleSwap"	// Data Name of swap slidingrule
#endif
#ifndef IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY
#define IR_CALIBRATION_DATA_GENERATEFORWARDSFROMSWAPSONLY "GenerateForwardsFromSwapsOnly" // data name of GenerateForwardsFromSwapsOnly
#endif
#ifndef IR_CALIBRATION_DATA_SHORTTERMSWAPOVERRULES
#define IR_CALIBRATION_DATA_SHORTTERMSWAPOVERRULES		"ShortTermSwapOverrules"		//  data name of ShortTermSwapOverrules
#endif
#ifndef IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE
#define IR_CALIBRATION_DATA_ALWAYSCALCJOINDATE		"AlwaysCalcJoinDate"		//  data name of AlwaysCalcJoinDate
#endif
#ifndef IR_CALIBRATION_DATA_COMPOUNDING_FUNCTION
#define IR_CALIBRATION_DATA_COMPOUNDING_FUNCTION	 "CompoundingFunction"		//  data name of Compounding function
#endif


//#define FWD		        "FWDRATE"
//#define BOJ	            "BOJRATE"
//#define FEDFUNDRATE	    "FFRATE"
//#define FUTURE            "FUTURE"

// datatype
#ifndef ZERO
#define ZERO	"ZERORATE"
#endif
#ifndef PAR
#define PAR		"PARRATE"
#endif
#ifndef BASIS
#define BASIS	"BASISRATE"
#endif
#ifndef O_N
#define O_N		"O_N"
#endif
#ifndef T_N
#define T_N		"T_N"
#endif
#ifndef LIBOR
#define LIBOR		"LIBOR"
#endif
#ifndef FRA3M
#define FRA3M	"FRA3M"
#endif
#ifndef FRA6M
#define FRA6M	"FRA6M"
#endif
#ifndef FRA
#define FRA		"FRA"
#endif
#ifndef IMMFRA
#define IMMFRA		"IMMFRA"
#endif
#ifndef FUTURE
#define FUTURE		"FUTURE"
#endif
#ifndef SWAP
#define SWAP		"SWAP"
#endif
#ifndef BOJ
#define BOJ			"BOJRATE"
#endif
#ifndef FEDFUNDRATE
#define FEDFUNDRATE		"FFRATE"
#endif
#ifndef FWDFX
#define FWDFX	"FWDFX"
#endif
#ifndef FWDFXCONST
#define FWDFXCONST	"FWDFXCONST"
#endif
#ifndef NDF
#define NDF	"NDF"
#endif

//arb free
#ifndef AF3ML 
#define AF3ML "AF3ML"
#endif
#ifndef AF6ML
#define AF6ML "AF6ML"
#endif
#ifndef AFDF
#define AFDF "AFDF"
#endif

#ifndef IR_NO_DATA
#define IR_NO_DATA "NO_DATA"
#endif

#if !defined(DAILYCOMPOUNDING)
#define DAILYCOMPOUNDING "DAILYCOMPOUNDING"
#endif
#if !defined(DAILYAVERAGING)
#define DAILYAVERAGING "DAILYAVERAGING"
#endif
#if !defined(FLATCOMPOUNDING)
#define FLATCOMPOUNDING "FLATCOMPOUNDING"
#endif


class AQLDataInstance;
class AQLDate;
class AQLDataDoubles;
class AQLDataDouble;
class AQLDataDate;
class AQLInterpolationBase;
class AQLCoreProcedure;
class AQLDataString;
class AQLDataStrings;
class AQLPriceDataCalendar;
class AQLPriceDataInterpolation;
class AQLPriceDataFunction;
class AQLDataMultiReference;
class AQLDataDoubleMatrix;
class AQLDataBool;
class AQLPriceDataInterpolation;
class AQLDataInts;

enum FloorType {
	NOFLOOR,	
	NORMAL,		
	SYMMETRIC	
};



//=================== YIELD CURVE ==================================
/*! 
    @brief Class of YieldCurve
*/

class AQLMathYieldCurvePro : public AQLMathYieldCurve
{
public:
// LIFECYCLE
	// constructor
	AQLMathYieldCurvePro(AQLDataInstance* dataInstance);
	// copy constructor
	AQLMathYieldCurvePro(const AQLMathYieldCurvePro& curve);
	// destructor
	virtual ~AQLMathYieldCurvePro();

//  QUERY
	// get EntityType
	virtual object_t	getType(void) const;
    // this object have specified object or not
	virtual bool		isTypeOf(object_t id) const;

	// get AsOfDate
	const AQLDataDate&	getAsOfDate(void) const;
    // get AsOfDate
	AQLDataDate&			getAsOfDate(void);
	// get IsArbFree
	const AQLDataBool&	getIsArbFree(void) const;
    // get AsOfDate
	AQLDataBool&			getIsArbFree(void);
	//	get MarketData
	const AQLDataMultiReference&
						getMarketData() const;
	//	get MarketData
	AQLDataMultiReference&
						getMarketData();
    // get BasisRates
	const AQLDataDoubleMatrix&
						getBasisRates(void) const;
    // get BasisRates
	AQLDataDoubleMatrix&		getBasisRates(void);
	// get BasisData
	const AQLDataMultiReference&
						getBasisData() const;
    // get BasisData
	AQLDataMultiReference&
						getBasisData();
	// get IsFutureUse
	const AQLDataBool&	getIsFutureUse(void) const;
    // get IsFutureUse
	AQLDataBool&			getIsFutureUse(void);
	// get IsFRAUse
	const AQLDataBool&	getIsFRAUse(void) const;
    // get IsFRAUse
	AQLDataBool&			getIsFRAUse(void);
	// get BaseYieldCurve
	const AQLDataReference&
						getBaseYieldCurve() const;
    // get BaseYieldCurve
	AQLDataReference&
						getBaseYieldCurve();
	//	get Interpolation to generate yield 
	const AQLPriceDataInterpolation&
						getInterpolation_yg() const;
	//	get Interpolation to generate yield 
	AQLPriceDataInterpolation&
						getInterpolation_yg();	
	//	get Interpolation to use forward 
	const AQLPriceDataInterpolation&
						getInterpolation_fw() const;
	//	get Interpolation to use forward
	AQLPriceDataInterpolation&	
						getInterpolation_fw();	
	//	get Interpolation to set basis rates 
	const AQLPriceDataInterpolation&
						getInterpolation_bs() const;
	//	get Interpolation to set basis rates
	AQLPriceDataInterpolation&	
						getInterpolation_bs();	
	//	get Basis Function
	const AQLPriceDataFunction&
						getBasisFunction() const;
	//	get getBasisFunction
	AQLPriceDataFunction&		getBasisFunction();
	
	AQLDataStrings&		getRatePriority();

	const AQLDataStrings&		getRatePriority() const;
	
	// get fx object
	const AQLDataReference&
						getFXEntity() const;
	// get fx object. The setting of fx object is also possible. 
	AQLDataReference&
						getFXEntity();	

	// get optimize method
	const AQLDataString&
						getOptimizeMethod() const;
	// get optimize method. The setting of optimize method is also possible. 
	AQLDataString&
						getOptimizeMethod();	

	// get optimize method
	const AQLPriceDataFunction&
						getCompoundingFunction() const;
	// get optimize method. The setting of optimize method is also possible. 
	AQLPriceDataFunction&
						getCompoundingFunction();


	//	clone this class
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	// remove specified Data. If Data is nothing, do nothing.
    virtual void        remove(const AQLString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
	//
	
	//	set Interpolation
	void				setInterpolation(AQLInterpolationBase* a, 
											const AQLString& name);
	//	set Interpolation
	void				setInterpolation(const AQLString& name);
	//	set Generator class
	void				setDFGenerator(const AQLCoreProcedure* a,const AQLString& name);
	//	set Generator class
	void				setDFGenerator(const AQLString& name);
	//	culc DF from marketData
	void				calcDiscountFactor(const AQLDate& asof);
	//	Set DF By ShiftZero
	void				setDFByShiftZero(double width, unsigned int pos, const UintArray& grids);
	//	Set DF By parallel ShiftZero
	void				setDFByShiftZero(double width, FloorType=NORMAL, double floor=0.00001);
	//	Set BasisDF By ShiftZero
	void				setBasisDFByShiftZero(double width, unsigned int pos, const UintArray& grids);
	//	Set BasisDF By parallel ShiftZero
	void				setBasisDFByShiftZero(double width, FloorType=NORMAL, double floor=0.00001);

	//	Set Basis rate
	void				setBasisRates(void);
	//	Set Basis rate
	void				setBasisRates2(const AQLString& basisCurveID);

	void				setBasisRates(const AQLString &curveType);
	//	Set Basis rate
	void				setBasisRates_old(void);
	//	Set DF2
	void				setDF2(void);

	//	Set Basis rate
	void				setBasisRates(const DoubleMatrix& values);

	void				addBasisRates(void);

	// Set ArbFree Curve Flag
	void				setArbFreeCurveName(const AQLStringVector& curveNames_6ML, 
											const AQLStringVector& curveNames_DF, 
											const AQLStringVector& curveNames_3ML);
	// Set dNPV/dm
	void				setdNPVdm(const AQLString &curveType);
	// get conversion matirx
	const AQLDataDoubleMatrix&	getConversionMatrix(const AQLString &curveType) const;
	// get conversion matirx term
	const AQLDataDoubles&	getConversionMatrixTerm(const AQLString &curveType) const;
	// get conversion matirx rates
	const AQLDataDoubles&	getConversionMarketRates(const AQLString &curveType) const;
	// get conversion matirx term types
	const AQLDataStrings&	getConversionMatrixTermTypes(const AQLString &curveType) const;
	// get conversion matirx term types
	const AQLDataStrings&	getConversionMarketTypes(const AQLString &curveType) const;

	// isOmitGridsExist
	bool					isOmitGridsExist(const AQLString &curveType) const;
	// getConversionOmitGrids
	const AQLDataInts&	getConversionOmitGrids(const AQLString &curveType) const;
	// setCurveDependencyMap
	void				setCurveDependencyMap(void);
	// getCurveDependeny
	std::map<AQLString, double>& getCurveDependeny(const AQLString &curveType) const ;
	// setGCurveGenerateMap
	void setGCurveGenerateMap(const AQLString &curveType){mGCurveGenMap[curveType] = true;};
	// set affecting currency
	void setAffectingCcy(const AQLString &currency)
	{
		if (std::find(mAffectingCcys.begin(), mAffectingCcys.end(), currency) == mAffectingCcys.end()) 
			mAffectingCcys.push_back(currency);
	};
	// set affected currency
	void				setAffectedCcy(const AQLString &currency){mAffectedCcy = currency;};
	// isBasisCurve
	bool				isBasisCurve(const AQLString &curveType) { return (mGCurveGenMap.find(curveType) == mGCurveGenMap.end());};
	// getBCurveGenerateMap
	const std::map<AQLString, bool>& getBCurveGenerateMap(void) const {return mBCurveGenMap;};
	// getGCurveGenerateMap
	const std::map<AQLString, bool>& getGCurveGenerateMap(void) const {return mGCurveGenMap;};
	// getGCurveGenerateMap
	std::map<AQLString, bool>& getGCurveGenerateMap(void){return mGCurveGenMap;};
	// get affecting currency
	const AQLStringVector& getAffectingCcy(void) const {return mAffectingCcys;};
	// get affected currency
	const AQLString& getAffectedCcy(void) const {return mAffectedCcy;};
	// changeZeroRiskIntoMarketRisk
	void changeZeroRiskIntoMarketRisk(AQLString curveType, const DoubleVector& termZeroVals, const DoubleVector& riskZeroVals,
												AQLStringVector& termMarketGrids, DoubleVector& riskMarketVals) const;
		// getGCurveGenerateMap
	void				clearGCurveGenerateMap(void) const {mGCurveGenMap.clear();};

		// getGCurveGenerateMap
	void				clearBCurveGenerateMap(void) const {mBCurveGenMap.clear();};
	// get mAssignedCurveMktMap
	AQLString getMarketForCurve(const AQLString &curveName) const
	{
		if (mAssignedCurveMktMap.find(curveName) == mAssignedCurveMktMap.end())	return IR_NO_DATA;
		else return mAssignedCurveMktMap[curveName];
	};
	// get mAssignedCurveMktMap
	const std::map<AQLString, AQLString>& getAssignedCurveMktMap() const {return mAssignedCurveMktMap;};
	// set mAssignedCurveMktMap
	void setAssignedCurveMktMap(const AQLString &curveName, const AQLString &mktName){mAssignedCurveMktMap[curveName] = mktName;};
	// get IsSwapTenorCahnge
	const AQLDataBool&	getIsSwapTenorAdjust(void) const;
    // get IsSwapTenorCahnge
	AQLDataBool&			getIsSwapTenorAdjust(void);
	// get ForeignYieldData
	const AQLDataReference&	
						getForeignYieldData() const;
    // get ForeignYieldData
	AQLDataReference&	getForeignYieldData();
	// calc floater PV
	void setFloater(const AQLString& curveName);
	// check whether attricutes of curves exist or not
	bool checkCurveAttr(const AQLString& mktName) const;
	// remove all curve data
	void removeAllCuveData(AQLObject &yieldData) const;
	// remove curve data
	void removeCuveData(AQLObject &yieldData, const AQLString& mktName) const;
	// remove basis curve data
	void removeBasisCuveData(AQLObject &yieldData) const;
	// get forward convention
	void getForwardConvention(const AQLString &curveName, AQLPriceDataDayCount &dc, AQLPriceDataSlidingRule &sld, AQLPriceDataCalendar &cal, AQLString &accessary) const;
	// insert non removable market
	void insertNonRemovableMarket(const AQLString& mktName);
	// erase non removable market
	void eraseNonRemovableMarket(const AQLString& mktName);

	// calc fwdfx constant curve
	void calcFwdFXConstantCurve(void);
	// calc fwdfx constant curve
	void calcFwdFXConstantCurve(const AQLString &curveType);
	// get colateral YieldData
	const AQLDataReference& getColYieldData() const;
    // get colateral YieldData
	AQLDataReference& getColYieldData();
	// get colateral affecting currency
	const AQLStringVector& getColAffectingCcy(void) const {return mColAffectingCcys;};
	// set colateral affecting currency
	void setColAffectingCcy(const AQLString &currency)
	{
		if (std::find(mColAffectingCcys.begin(), mColAffectingCcys.end(), currency) == mColAffectingCcys.end()) 
			mColAffectingCcys.push_back(currency);
	};
	// get colateral affected currency
	const AQLString& getColAffectedCcy(void) const {return mColAffectedCcy;};
	// set colateral affected currency
	void setColAffectedCcy(const AQLString &currency){mColAffectedCcy = currency;};

	
protected:
	// copy		 
	virtual AQLObject&	copy(const AQLObject& e);
	//	calc Basis rate
	void				calcBasisDiscountFactor(const AQLString &type, const DoubleArray &dfs_base, DoubleArray &dfs_mod, DoubleArray &yield_mod, bool isSetAttr = true, const DoubleArray *pBasis = 0);
private:
	//	calc Index grid
	void				calcIndexGrid(const AQLDate &asofdate, const AQLDate &date, int resetLag, const AQLPriceDataDayCount &dc, const AQLPriceDataCalendar &fixcal, const AQLPriceDataCalendar &paycal, const AQLPriceDataSlidingRule &sld, const AQLString &freq, const AQLString &accessary, 
										DoubleVector &gridVec, DoubleVector &termVec);
	//	calc basis cashflow
	void calcBasisCF(const AQLPriceDataInterpolation &s_inter, const bool isDiscount, const bool isFwdRen, const bool isUSD, const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, const std::vector<DoubleArray> &dfsVec, 
	    const AQLPriceDataInterpolation &a_f_inter, const AQLPriceDataInterpolation *a_fwd_inter, const AQLPriceDataInterpolation &a_d_inter, const double a_d_df_adjust, const AQLPriceDataInterpolation *adjust_inter, 
	    const bool isAgtSpread, const DoubleArray &spreadVec, const std::vector<IntArray> &cpd_timesVec, const std::vector<IntArray> &a_cpd_timesVec, const double spotTerm, const double a_spotTerm, 
	    const std::vector<DoubleArray> &gridVec, const std::vector<DoubleArray> &tauVec, const std::vector<DoubleMatrix> &i_gridMatVec, const std::vector<DoubleMatrix> &i_termMatVec, 
	    const std::vector<DoubleArray> &a_gridVec, const std::vector<DoubleArray> &a_tauVec, const std::vector<DoubleMatrix> &a_i_gridMatVec, const std::vector<DoubleMatrix> &a_i_termMatVec, 
	    const std::vector<DoubleMatrix> &b_yieldTimeMatVec, DoubleArray &a_targetPVVec, DoubleArray &out, const bool isNegative);
	// calc against PV (spot renotional)
	double					
	calcAgainstPV(const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, 
		const AQLPriceDataInterpolation &d_inter, const double spread, const IntArray& cpd_times, const double term_spot, 
		const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, const bool isBackward);
	// calc against PV (forward renotional)
	double					
	calcAgainstPV(const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, 
		const AQLPriceDataInterpolation &d_inter, const AQLPriceDataInterpolation &a_s_inter, const AQLPriceDataInterpolation *adjust_inter, const double spread, const IntArray& cpd_times, const double term_spot, const double a_term_spot, 
		const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat);
	// calc target PV for Newton-Raphson method to generate Discount Curve (spot renotional)
	double
	calcTargetPV(const AQLPriceDataInterpolation &s_inter, const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, const double spread, const IntArray& cpd_times, 
		const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
		const DoubleMatrix &b_yieldTimeMat);
	// calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)
	double
	calcTargetPV(const AQLPriceDataInterpolation &s_inter, const bool isFWDInter, const AQLPriceDataInterpolation &f_inter, const AQLPriceDataInterpolation *fwd_inter, const AQLPriceDataInterpolation &a_d_inter, const AQLPriceDataInterpolation *adjust_inter, const double spread, const IntArray& cpd_times, 
		const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat,
		const DoubleMatrix &b_yieldTimeMat);
	// calc target PV for Newton-Raphson method to generate Forecast Curve
	double	
	calcTargetPV(const AQLPriceDataInterpolation &s_inter, const DoubleArray &dfsVec, const double spread, const IntArray& cpd_times, 
		const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
		const DoubleMatrix &b_yieldTimeMat);
	// claculate present value of cashflows in specified leg.
	double
	calcLegPV(const DoubleArray &fwdsVec, const DoubleMatrix &dfsMat, const DoubleArray &a_dfsMat, const AQLPriceDataInterpolation *adjust_inter,
		const double spread, const IntArray& cpd_times, const DoubleArray &terms_grid, const DoubleArray& terms_interval, const DoubleMatrix &i_gridMat);

	//	set a rate convention into a curve data object
	void				setCurveConvention(AQLObjectHolder& objHolder, std::vector<AQLObject*>& mktData, const AQLString& curveName);

	// get market data reference
	const AQLDataMultiReference& getMarketDataRef(const AQLString& curveType) const;
	//	save basis curve
	void saveBasisCurve(const AQLString& curveType, const DoubleArray& terms, const DoubleMatrix& termsmtx_fwd, const DoubleArray& dfs, std::vector<AQLObject*>& data);
	// set curve interpolation
	void setCurveInterpolation(const AQLString& curveName, const AQLObject &yieldData, AQLPriceDataInterpolation& inter) const;
	// calc annuity
	static double calcAnnuity(double spotTerm, const DoubleArray& terms_grid, const DoubleArray& terms_interval, const AQLPriceDataInterpolation& d_inter);
	// deduce dates and terms generally
	static void deducePaymentDatesAndTerms(const AQLDate& spotdate, const AQLString& strTerm, const AQLPriceDataSlidingRule& sld, const AQLPriceDataCalendar& cal,
		const AQLString& roll_conv, const AQLString& freq, const AQLString& freq_payment, const AQLPriceDataDayCount& dc, bool eom, bool isBackward,
		DateVector& dates, DoubleArray& terms_grid, DoubleArray& terms_interval, IntArray& num_reset);
	// deduce dates and terms of the leg of this side reflecting the against-side leg info
	static void deducePaymentDatesAndTermsThisSide(const AQLString& strTerm, const AQLString& roll_conv, bool eom, bool isSameGridIndex,
		const AQLDate& c_spotdate, const AQLPriceDataSlidingRule& c_sld, const AQLPriceDataCalendar& c_cal, const AQLString& c_freq, const AQLString& c_freq_payment, const AQLPriceDataDayCount& c_dc, bool c_isBackward, const AQLDate& a_c_spotdate, 
		DateVector& dates, DoubleArray& terms_grid, DoubleArray& terms_interval, IntArray& num_reset);

	// Set data by name
	AQLDataHolder&		add(const AQLString& name);
    AQLDataHolder&		reset(const AQLString& name); // remove then add
	
	AQLDataHolder*       mpAsOfDate;			// AsOfDate							(DATA_DATE)
	AQLDataHolder*		mpProcedure;		// Procedure to genarate DF			(DATA_PROCEDURE) 
	AQLDataHolder*		mpMarketData;		// Market Data						(DATA_MULTIREFERENCE) 
	AQLDataHolder*       mpBasisRates;		// BasisRates of Terms				(DATA_DOUBLE_MATRIX)
	AQLDataHolder*		mpBasisData;		// BasisData							(DATA_MULTIREFERENCE) 
	AQLDataHolder*		mpIsFutureUse;		// Use FutureRate or not				(DATA_BOOL) 
	AQLDataHolder*		mpBaseYieldCurve;	// BaseYieldCurve						(DATA_REFERENCE) 
	AQLDataHolder*		mpCurrency;			// Currency							(DATA_STRING)
	AQLDataHolder*		mpInterYG;			// interpolation to getnerate yeild	(DATA_PROCEDURE) 
	AQLDataHolder*		mpInterFW;			// interpolation to use foward		(DATA_PROCEDURE) 
	AQLDataHolder*		mpInterBS;			// interpolation to use basis			(DATA_PROCEDURE) 
	AQLDataHolder*		mpBasisFunction;	// Basis Function						(DATA_FUNCTION)
	AQLDataHolder*		mpInterOC;			// interpolation to optional curve	(DATA_PROCEDURE) 
	AQLDataHolder*		mpRatePriority;		// interpolation to use basis			(DATA_STRINGS)
	AQLDataHolder*		mpIsArbFree;		// ArbFree Flag						(DATA_BOOL)
	AQLDataHolder*		mpIsFRAUse;			// FRA Flag							(DATA_BOOL)
	AQLDataHolder*		mpIsSwapTenorAdjust;// Swap Tenor Change Flag				(DATA_BOOL)
	AQLDataHolder*		mpFXEntity;			// FXEntity							(DATA_REFERENCE) 
	AQLDataHolder*		mpOptimizeMethod;	// Name of Optimization Method		(DATA_STRING) 
	AQLDataHolder*       mpCompoundFunction; // Compounding function               (DATA_FUNCTION)
	mutable std::map<AQLString, bool>  mBCurveGenMap;  // BasisCurveGenerateMap
	std::map<AQLString, bool>  mArbFreeCurveGenMap;  // ArbFreeCurveGenerateMap
	mutable std::map<AQLString, bool> mGCurveGenMap; // GenerateCurveGenMap
	std::map<AQLString, std::map<AQLString, double> >mDpnCurveMap; 
	mutable AQLString mBfCurveType;
	mutable std::map<AQLString, double> mBfDpnMap;
	AQLDataHolder*		mpForeignYieldData;	// Foreign currency YieldData			(DATA_REFERENCE)
	mutable std::map<AQLString, AQLString> mAssignedCurveMktMap; // map(curve name, market name)
	AQLStringVector mAffectingCcys;
	AQLString mAffectedCcy;
	StringSet mNonRemovableMarket; // Non removable market

	AQLDataHolder*		mpColYieldData;	// Collateral currency YieldData			(DATA_REFERENCE)
	AQLStringVector mColAffectingCcys;	// Collateral affecting currency
	AQLString mColAffectedCcy;	// Collateral  affected currency

	class CalibrationCostFunction : public AQLFunctionVector
	{
	public: 
		CalibrationCostFunction(AQLMathYieldCurvePro& outerObject,
                                const DoubleArray grid_spread_time,
                                AQLPriceDataInterpolation& spread_time_inter,
                                const DoubleVector targetPV,
								unsigned int maxIterationNum = 150
							   );
		CalibrationCostFunction(const CalibrationCostFunction& rhs);
		~CalibrationCostFunction();
		//paramater accessors
		unsigned long lengthOfArgumentVector() { return mGrid_spread_time.size() - 1; }
		unsigned long lengthOfFunctionVector() { return mTargetPV.size(); }
		unsigned long maximumNumberOfIterations() { return mMaxIterationNum; }
		//cost function
		virtual void operator()(DoubleArray& f, const DoubleArray& x) = 0;
		//constraint function
		bool constraintsAreViolated(const DoubleArray& x);
		//constraint 
	protected:
		AQLMathYieldCurvePro& mParent;
		DoubleArray mGrid_spread_time;
		AQLPriceDataInterpolation& mSpread_time_inter;
		DoubleVector mTargetPV;
		unsigned int mMaxIterationNum;
	};

	class CalibrationCostFunctionBasis : public CalibrationCostFunction
	{
	public: 
		CalibrationCostFunctionBasis(AQLMathYieldCurvePro& outerObject,
                                     const DoubleArray grid_spread_time,
                                     AQLPriceDataInterpolation &s_inter,
                                     const bool isDiscount, 
                                     const bool isFwdRen, 
                                     const bool isUSD, 
                                     const bool isFWDInter, 
                                     const AQLPriceDataInterpolation &f_inter, 
                                     const AQLPriceDataInterpolation *fwd_inter, 
                                     const std::vector<DoubleArray> &dfsVec, 
	                                 const AQLPriceDataInterpolation &a_f_inter, 
                                     const AQLPriceDataInterpolation *a_fwd_inter, 
                                     const AQLPriceDataInterpolation &a_d_inter, 
                                     const double a_d_df_adjust, 
                                     const AQLPriceDataInterpolation *adjust_inter, 
	                                 const bool isAgtSpread, 
                                     const DoubleArray &spreadVec, 
                                     const std::vector<IntArray> &cpd_times, 
                                     const std::vector<IntArray> &a_cpd_times, 
                                     const double spotTerm, 
                                     const double a_spotTerm, 
	                                 const std::vector<DoubleArray> &gridVec, 
                                     const std::vector<DoubleArray> &tauVec, 
                                     const std::vector<DoubleMatrix> &i_gridMatVec, 
                                     const std::vector<DoubleMatrix> &i_termMatVec, 
	                                 const std::vector<DoubleArray> &a_gridVec, 
                                     const std::vector<DoubleArray> &a_tauVec, 
                                     const std::vector<DoubleMatrix> &a_i_gridMatVec, 
                                     const std::vector<DoubleMatrix> &a_i_termMatVec, 
	                                 const std::vector<DoubleMatrix> &b_yieldTimeMatVec, 
                                     DoubleArray &a_targetPVVec, 
                                     DoubleArray &targetPV, 
                                     const bool isNegative,
									 unsigned int maxIterationNum = 150
									);
		CalibrationCostFunctionBasis(const CalibrationCostFunctionBasis& rhs);
		~CalibrationCostFunctionBasis();
		//cost function
		virtual void operator()(DoubleArray& f, const DoubleArray& x);
	private:
		const bool mIsDiscount;
        const bool mIsFwdRen;
        const bool mIsUSD;
        const bool mIsFWDInter;
        const AQLPriceDataInterpolation &mF_inter;
        const AQLPriceDataInterpolation *mpFwd_inter;
        const std::vector<DoubleArray> &mDfsVec;
	    const AQLPriceDataInterpolation &mA_f_inter;
        const AQLPriceDataInterpolation *mpA_fwd_inter; 
        const AQLPriceDataInterpolation &mA_d_inter;
        const double mA_d_df_adjust;
        const AQLPriceDataInterpolation *mpAdjust_inter;
	    const bool mIsAgtSpread;
        const DoubleArray &mSpreadVec;
        const std::vector<IntArray> mCpd_times;
        const std::vector<IntArray> mA_cpd_times;
        const double mSpotTerm;
        const double mA_spotTerm;
	    const std::vector<DoubleArray> &mGridVec;
        const std::vector<DoubleArray> &mTauVec;
        const std::vector<DoubleMatrix> &mI_gridMatVec;
        const std::vector<DoubleMatrix> &mI_termMatVec;
	    const std::vector<DoubleArray> &mA_gridVec;
        const std::vector<DoubleArray> &mA_tauVec;
        const std::vector<DoubleMatrix> &mA_i_gridMatVec;
        const std::vector<DoubleMatrix> &mA_i_termMatVec;
	    const std::vector<DoubleMatrix> &mB_yieldTimeMatVec;
        DoubleArray &mA_targetPVVec;
        const bool mIsNegative;
	};
};

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Funciton ID of AQLShiftMethod
#define FN_BASISFUNC1	2700
// Function Name of AQLShiftMethod
#define FN_BASISFUNC1_STR	"fn_basisfunc1"


class AQLMathBasisFunction : public AQLFunctionBase
{
public:
	// Default constructor
	AQLMathBasisFunction();
	// Destructor
	~AQLMathBasisFunction();
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	virtual double				operator()(const DoubleArray& x) const;
	virtual double				operator()(const double& x) const;

	// 
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								}
	//
	AQLMathBasisFunction & operator=( const AQLMathBasisFunction & ) { return *this; }
	
private:
};


#ifdef __GNUG__
#pragma interface
#endif


#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Funciton ID of AQLShiftMethod
#define FN_BASISFUNC2	2701
// Function Name of AQLShiftMethod
#define FN_BASISFUNC2_STR	"fn_basisfunc2"


class AQLMathBasisFunction2 : public AQLFunctionBase
{
public:
	// Default constructor
	AQLMathBasisFunction2();
	// Destructor
	~AQLMathBasisFunction2();
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%

	virtual double				operator()(const DoubleArray& x) const;
	virtual double				operator()(const double& x) const;

	// 
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								}
	//
	AQLMathBasisFunction2 & operator=( const AQLMathBasisFunction2 & ) { return *this; }
	
private:
};

class Comp_term
{
public:
	/*!
		@brief compare term
		@param[in] _Left one object
		@param[in] _Right another object
		@return true when right argument maturity date > left argument maturity date
	*/
	bool operator()(const AQLObject* _Left, const AQLObject* _Right) const
	{
		AQLDate asof, ldate, rdate;
		AQLString dataType = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
		dataType.toUpper(); 

		if (dataType == FRA3M || dataType == FRA6M || dataType == FRA)
		{
			AQLString lterm_x = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLString rterm_x = dynamic_cast<const AQLDataString&> ((_Right->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLString lterm = AQLPriceYieldGenerator::changeFRATermFormat(lterm_x);
			AQLString rterm = AQLPriceYieldGenerator::changeFRATermFormat(rterm_x);
			asof.setSystemDate();
			ldate = AQLMathDateCalculations::getDate(asof, lterm, true);
			rdate = AQLMathDateCalculations::getDate(asof, rterm, true);
			return ldate < rdate;
		}
		else if(dataType != FUTURE && dataType != BOJ && dataType != FEDFUNDRATE)
		{
			AQLString lterm = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLString rterm = dynamic_cast<const AQLDataString&> ((_Right->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			asof.setSystemDate();
			ldate = AQLMathDateCalculations::getDate(asof, lterm, true);
			rdate = AQLMathDateCalculations::getDate(asof, rterm, true);
			return ldate < rdate;
		}

		ldate = dynamic_cast<const AQLDataDate&> ((_Left->getData(IR_CALIBRATION_DATA_STARTDATE, ISNOTNULL)).get()).get();
		rdate = dynamic_cast<const AQLDataDate&> ((_Right->getData(IR_CALIBRATION_DATA_STARTDATE, ISNOTNULL)).get()).get();
		return ldate < rdate;
	};
};
