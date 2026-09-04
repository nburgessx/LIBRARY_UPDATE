/*! @file
    @brief Define of Data name

    Data name defines the various types. Data names are defined with a name beginning with CALIBRATION_DATA_.

*/


#ifndef AQLDefine_h
#define AQLDefine_h

#ifdef __GNUG__
#pragma interface
#endif

#define CALIBRATION_DATA_NAME			    "Name"				// Data of Name

#define CALIBRATION_DATA_ASOFDATE         "AsOfDate"			// Data AsOfDate

#define CALIBRATION_DATA_TRADEDATE        "TradeDate"			// Data TradeDate

#define CALIBRATION_DATA_MATURITY         "Maturity"			// Data Maturity

#define CALIBRATION_DATA_NOTIONAL         "Notional"			// Data Notional

#define CALIBRATION_DATA_UNDERLYING		"Underlying"		// Data Underlying

#define CALIBRATION_DATA_UNDERLYINGS		"Underlyings"		// Data Underlyings

#define CALIBRATION_DATA_VALUE			"Value"				// Data Value

#define CALIBRATION_DATA_PROCEDURE		"Procedure"			// Data Procedure

#define CALIBRATION_DATA_INTERPOLATION	"Interpolation"		// Data Interpolation

#define CALIBRATION_DATA_CALENDAR			"Calendar"			// Data Calendar

#define CALIBRATION_DATA_SLIDINGRULE		"SlidingRule"		// Data SlidingRule

#define CALIBRATION_DATA_VALUEDATE        "ValueDate"			// Data ValueDate

#define CALIBRATION_DATA_FEEAMOUNTS       "FeeAmounts"		// Data FeeAmounts

#define CALIBRATION_DATA_FEEPAYMENTDATES  "FeePaymentDates"	// Data FeePaymentDates

#define CALIBRATION_DATA_FEECURRENCIES    "FeeCurrencies"		// Data FeeCurrencies

#define CALIBRATION_DATA_FEEDISCOUNTCURVES    "FeeDiscountCurves"		// Data FeeCurrencies

// For YieldCurve
#define CALIBRATION_DATA_TERMS			"Terms"				// Data Terms

#define CALIBRATION_DATA_FWDTERMSMATRIX	"FwdTermsMatrix"	// Data FwdTermsMatrix

#define CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE	"InterpolationJoinDateAsDouble"	// Data InterpolationJoinDateAsDouble

#define CALIBRATION_DATA_INTERPOLATION_JOINDATE	"InterpolationJoinDate"	// Data InterpolationJoinDate

#define CALIBRATION_DATA_CONVENTION		"Convention"		// Data Convention

#define CALIBRATION_DATA_CURVEGENERATOR   "CurveGenerator"	// Data CurveGenerator

#define CALIBRATION_DATA_ZERORATES		"ZeroRates"			// Data ZeroRates

#define CALIBRATION_DATA_MARKETDATANAMES	"MarketDataNames"	// Data MarketDataNames

#define CALIBRATION_DATA_MARKETDATA		"MarketData"		// Data MarketData

#define CALIBRATION_DATA_MARKETDATA_EXT	"MarketDataExt"		// Data External MarketDate

#define CALIBRATION_DATA_CURVETYPE		"CurveType"			// Data CurveType

#define CALIBRATION_DATA_BASISCURVEBASE	 "BasisCurveBase"	// Data BasisCurveBase

#define CALIBRATION_DATA_BASISCURVESECONDARYBASE	 "BasisCurveSecondaryBase"	// Data BasisCurveSecondaryBase

#define CALIBRATION_DATA_XCCYSAMESIDEFORECASTCURVE "XccySameSideForecastCurve" // Data XccySameSideForecastCurve

#define	CALIBRATION_DATA_RATE				"Rate"				// Data Rate

// For Volatility
#define CALIBRATION_DATA_1DVOLATILITY		"1DVolatility"		// Data 1DVolatility

// For Correlation
#define CALIBRATION_DATA_AVGECORRELATION	"AvgeCorrelation"	// Data AvgeCorrelation

#define CALIBRATION_DATA_INSTCORRELATION	"InstCorrelation"	// Data InstCorrelation

// For LAMATHScenarioEntity
#define IR_CALIBRATION_DATAMODEL				"IRModel"		// Data IRModel

#define CALIBRATION_DATA_SCENARIONAMES		"ScenarioNames"	// Data ScenarioNames

#define CALIBRATION_DATA_SETTLEDATES			"SettleDates"   // Data SettleDates

// For MF1DTrinomialTree
#define CALIBRATION_DATA_MODEL				"Model"			// Data Model

#define	CALIBRATION_DATA_TREEDIVNUM			"TreeDivNum"	// Data TreeDivNum

// For AQLSingleCredit
//#define CALIBRATION_DATA_DAYCOUNT				"DayCount"		//> Data Name of DayCount

#define CALIBRATION_DATA_HAZARDRATE			"HazardRate"	// Data Name of HazardRate

#define CALIBRATION_DATA_RECOVERYRATE			"RecoveryRate"	// Data Name of RecoveryRate

// For AQLCreditPoolMC
#define CALIBRATION_DATA_RANDGENERATOR		"RandGenerator"	// Data Name of RandGenerator

#define CALIBRATION_DATA_DEFAULTSCENARIO	"DefaultScenario"	// Data Name of DefaultScenario	

// For AQLCreditPoolSA
#define CALIBRATION_DATA_BASECORRELATION		"BaseCorrelation"	// Data Name of 1-factor model base correlation

// For ErrorMessages
#define CALIBRATION_DATA_ERRORMESSAGES	"ErrorMessages"		// Data Name of ErrorMessages

#endif
