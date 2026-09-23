#pragma once

#include <string>
#include <vector>
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API

/* @brief			swig interface for aqCurveObjectDisplay
*  @param [in]		curveHandle		    AQObj curve handle or object name
*  @return			StandardStringMatrix of AQObj Curve static and market data inputs
*/
SWIG_STRINGMATRIX aqCurveObjectDisplay(const std::string& curveHandle);

/* @brief			swig interface for aqCurveMarketDataDisplay
*  @param [in]		marketDataObjectName	AQObj curve market data handle or object name
*  @param [in]		marketDataKey			The specific market data block of interest, for example "SWAPS"
*  @return			A string matrix containing the specified market data block.
*/
SWIG_STRINGMATRIX aqCurveMarketDataDisplay(const std::string& marketDataObjectName, const std::string marketDataKey );

/* @brief			swig interface for aqObjCurvesDisplayMarketDataFromCurve
*  @param [in]		curveObjectName			AQObj curve data handle or object name
*  @param [in]		marketDataKey			The specific market data block of interest, for example "SWAPS"
*  @return			A string matrix containing the specified market data block.
*/
SWIG_STRINGMATRIX aqCurveMarketDataDisplayFromCurve(const std::string& curveObjectName, const std::string marketDataKey );


/* @brief Builds a curve using a CurveGenerator object and a CurveMarketData object
* @param [in] aqObjCurveGeneratorName     The name of the AQObjCurveGenerator object to use
* @param [in] aqObjCurveMarketDataName    The name of the AQObjCurveMarketData object to use
* @param [in] domesticCurveCollection   The curveCollection this curve will be stored in ( The Target CurveCollection )
* @param [in] foreignCurveCollection    The curveCollection containing foreign curve dependencies ( The Against CurveCollection )
*                                       Allowed to be blank if this is a single CCY curve (OIS, STD, TenorBasis)
* @param [out]                          The curve build status
*/
std::string aqCurveObjectCalibrate( const std::string& objectName,
								 const std::string& aqObjCurveGeneratorName,
								 const std::string& aqObjCurveMarketDataName,
								 const std::string& domesticCurveCollection,
								 const std::string& foreignCurveCollection );


/* @brief Creates a AQObjCurveMarketData object, containing all of the curve properties.
* @param [in] objectName	The name of the Market Data object
* @param [in] key1			The name of datablock1 For example "MARKETDATAPROPERTIES"
* @param [in] value1		The contents of datablock1 
* @param [in] key2			The name of datablock2 For example "OIS"
* @param [in] value2		The contents of datablock2
* @param [in] key3			The name of datablock3 For example "LIBOROISBASISSPREADS"
* @param [in] value3		The contents of datablock3
* @param [in] key4			The name of datablock4 For example "SWAPS"
* @param [in] value4		The contents of datablock4
* @param [out]              The objectName
*/
std::string aqCurveMarketDataCreate( const std::string& objectName,
										const std::string& key1, const SWIG_STRINGMATRIX& value1,
										const std::string& key2, const SWIG_STRINGMATRIX& value2,
										const std::string& key3, const SWIG_STRINGMATRIX& value3,
										const std::string& key4, const SWIG_STRINGMATRIX& value4) ;


/* @brief Creates a AQObjCurveMarketData object, containing all of the curve properties.
*			NOTE: This function not work correctly in R because R cannot cope with vectors of vectors.
*			In particular the vector<SWIG_STRINGMATRIX> parameter is a problem in R.
* @param [in] objectName	The name of the Market Data object
* @param [in] keyVector		The names all the datablocks, for example "MARKETDATAPROPERTIES", "SWAPS", "FUTURES", ...
* @param [in] valueVector	The vector of datablocks
* @param [out]              The objectName
*/
std::string aqCurveMarketDataCreate( const std::string& objectName,
														   const std::vector<std::string>& keyVector,
														   const std::vector<SWIG_STRINGMATRIX>& valueVector );
										

/* @brief			function to create a fixing table object
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		currency	        Currency as a string
*  @param [in]		curveTenor	        The curve tenor string: ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY, DAILY ...
*  @param [in]		fixingDates	        Vector of Fixing Dates in String Format
*  @param [in]		fixingValues	    Vector of Fixing Values in Double Format
*  @return			returns the name of the fixing table on the cache
*/
std::string aqIRFixingTableCreate( const std::string& tableName,
                                    const std::string& currency,                                   
                                    const std::string& curveTenor,
                                    const std::vector< std::string >& fixingDates,
                                    const std::vector< double >& fixingValues );

/* @brief			function to display a fixing table object
*  @param [in]		tableName		    Fixing Table name
*  @return			returns a VariantMatrix representing the fixing currency, curveTenor, fixingDates and fixingValues
*/
SWIG_STRINGMATRIX aqIRFixingTableDisplay( const std::string& tableName );

/* @brief			function to get the fixing value for a particular date
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		fixingDate	        Fixing Date
*  @return			fixing value
*/
double aqIRFixingTableValue( const std::string& tableName, const std::string & fixingDate );

/* @brief			function to get fixing values for a vector of dates
*  @param [in]		tableName		    Fixing Table name
*  @param [in]		fixingDates     	Fixing Dates
*  @return			fixing values
*/
std::vector<double> aqIRFixingTableValues( const std::string& tableName, const std::vector< std::string >& fixingDates );

/* @brief			Validation method which calculates a set of discount factors for the specified dates.
*					If a spread is provided, calculates the discount factors using the zero discount-margin
*					approach described by O'Kane in "Credit Spreads Explained".
*
*  @param [in]		paymentDates	A vector of one or more dates
*  @param [in]		curveCollection	CurveCollection name
*  @param [in]		curveIndex		Index of the curve.
*  @param [in]		spread			Spread to be added to the curve zero rate when calculating the discount factor
*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.
*  @return			A vector of discount factors
*/
std::vector<double> aqCurveObjectDiscountFactorsWithSpread( const std::vector<std::string>& paymentDates, const std::string& curveCollection, const std::string& curveIndex, const double& spread, const std::string& fixingTableName );

/* -------------------------------------------------------------------------
 *  Object lifecycle
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectList
*  @return			The names of every cached curve
*/
std::vector<std::string> aqCurveObjectList();

/* @brief			swig interface for aqCurveObjectDelete
*  @param [in]		curveName		A curve handle
*  @return			TRUE on success
*/
bool aqCurveObjectDelete( const std::string& curveName );

/* @brief			swig interface for aqCurveObjectDeleteAll
*  @return			The number of curves removed
*/
int aqCurveObjectDeleteAll();

/* @brief			swig interface for aqCurveObjectSave
*  @param [in]		aqObjCurveName		A curve handle
*  @param [in]		fileNameToWriteTo	Full path to write the curve to
*  @return			A status string
*/
std::string aqCurveObjectSave( const std::string& aqObjCurveName, const std::string& fileNameToWriteTo );

/* @brief			swig interface for aqCurveObjectLoad
*  @param [in]		fileName		Full path to the curve file
*  @return			A status string
*/
std::string aqCurveObjectLoad( const std::string& fileName );

/* -------------------------------------------------------------------------
 *  Curve market data object ("ObjectData" LVB family)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectDataCreate: build curve market data from raw
*					swap/FRA/futures/central-bank/FX static data blocks.
*  @param [in]		mdcName					Name for the market-data object
*  @param [in]		currency				Currency
*  @param [in]		tenorString				Swap tenors to include
*  @param [in]		swapType				Swap instrument type (optional, may be blank)
*  @param [in]		swapStringBlock			Swap rates block (optional, may be empty)
*  @param [in]		toTenorString			FRA/futures end tenors (optional, may be blank)
*  @param [in]		fraStringBlock			FRA rates block (optional, may be empty)
*  @param [in]		irFuturesStringBlock	IR futures rates block (optional, may be empty)
*  @param [in]		centralBankTypeString	Central bank meeting type (optional, may be blank)
*  @param [in]		centralBankStringBlock	Central bank meeting dates block (optional, may be empty)
*  @param [in]		fxStringBlock			FX rates block (optional, may be empty)
*  @param [in]		unitCurrency			FX unit currency (optional, may be blank)
*  @param [in]		isInvertedFX			TRUE if the FX quote is inverted
*  @return			The name of the market-data object on the cache
*/
std::string aqCurveObjectDataCreate( const std::string& mdcName,
                                      const std::string& currency,
                                      const std::string& tenorString,
                                      const std::string& swapType,
                                      const SWIG_STRINGMATRIX& swapStringBlock,
                                      const std::string& toTenorString,
                                      const SWIG_STRINGMATRIX& fraStringBlock,
                                      const SWIG_STRINGMATRIX& irFuturesStringBlock,
                                      const std::string& centralBankTypeString,
                                      const SWIG_STRINGMATRIX& centralBankStringBlock,
                                      const SWIG_STRINGMATRIX& fxStringBlock,
                                      const std::string& unitCurrency,
                                      const bool isInvertedFX );

/* @brief			swig interface for aqCurveObjectDataDisplay
*  @param [in]		mdcName		A market-data object handle built via aqCurveObjectDataCreate
*  @return			The market-data object's inputs, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectDataDisplay( const std::string& mdcName );

/* -------------------------------------------------------------------------
 *  Curve object conventions / dual bootstrap / engine calibration
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectDisplayConventions
*  @param [in]		objectName		A curve handle
*  @param [in]		propertyKey		The convention block to display
*  @return			The curve object's resolved conventions, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectDisplayConventions( const std::string& objectName, const std::string& propertyKey );

/* @brief			swig interface for aqCurveObjectDualBootstrap: dual-bootstrap OIS and swap curves together
*  @param [in]		objectName				Name for the dual-bootstrapped object
*  @param [in]		curveCollection			Curve collection the calibrated curves belong to
*  @param [in]		swapCurveGeneratorName	A curve-generator handle defining the swap curve's conventions
*  @param [in]		oisCurveGeneratorName	A curve-generator handle defining the OIS curve's conventions
*  @param [in]		aqObjSwapMarketObj		A curve-market-data handle for the swap curve
*  @param [in]		aqObjOISMarketObj		A curve-market-data handle for the OIS curve
*  @param [in]		commonParams			Parameters common across both curves
*  @return			The resulting curve index names, as a key/value string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectDualBootstrap( const std::string& objectName,
                                               const std::string& curveCollection,
                                               const std::string& swapCurveGeneratorName,
                                               const std::string& oisCurveGeneratorName,
                                               const std::string& aqObjSwapMarketObj,
                                               const std::string& aqObjOISMarketObj,
                                               const SWIG_STRINGMATRIX& commonParams );

/* @brief			swig interface for aqCurveObjectCalibrateHedge: calibrate a hedge-curve pairing
*					(OIS discounting + Libor forwarding).
*  @param [in]		oisCurveObjectName		Name for the OIS discount curve
*  @param [in]		swapCurveObjectName		Name for the swap forward curve
*  @param [in]		pricingCurveCollection	Curve collection to price the hedge instruments off
*  @param [in]		hedgeCurveCollection	Curve collection the hedge curves are stored under
*  @param [in]		oisCurveGeneratorName	A curve-generator handle for the OIS curve
*  @param [in]		oisCurveMarketDataName	A curve-market-data handle for the OIS curve
*  @param [in]		swapCurveGeneratorName	A curve-generator handle for the swap curve
*  @param [in]		swapCurveMarketDataName	A curve-market-data handle for the swap curve
*  @param [in]		swapGeneratorName		A swap-generator handle for the hedge instruments
*  @return			The hedge curve names/indices, as a key/value string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectCalibrateHedge( const std::string& oisCurveObjectName,
                                                const std::string& swapCurveObjectName,
                                                const std::string& pricingCurveCollection,
                                                const std::string& hedgeCurveCollection,
                                                const std::string& oisCurveGeneratorName,
                                                const std::string& oisCurveMarketDataName,
                                                const std::string& swapCurveGeneratorName,
                                                const std::string& swapCurveMarketDataName,
                                                const std::string& swapGeneratorName );

/* @brief			swig interface for aqCurveObjectEngineCalibrate: calibrate a global yield curve
*					engine, producing several synchronous curves at once.
*  @param [in]		engineObjectName	Name for the curve engine object
*  @param [in]		curveCollection		Curve collection the calibrated curves belong to
*  @param [in]		engineSettings		Engine-level parameters, as a label/value block
*  @param [in]		curveGeneratorNames	Curve-generator handles, one per curve
*  @param [in]		marketDataObjects	Curve-market-data handles, aligned with curveGeneratorNames
*  @return			The resulting curve index names
*/
std::vector<std::string> aqCurveObjectEngineCalibrate( const std::string& engineObjectName,
                                                         const std::string& curveCollection,
                                                         const SWIG_STRINGMATRIX& engineSettings,
                                                         const std::vector<std::string>& curveGeneratorNames,
                                                         const std::vector<std::string>& marketDataObjects );

/* -------------------------------------------------------------------------
 *  Curve object one-shot creation from raw conventions/rates
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectCreateBasis: build a cached cross-currency basis curve
*  @param [in]		aqObjCurveName		Name for the curve object
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		basisConv			Basis swap conventions
*  @param [in]		basisRates			Basis swap market rates
*  @param [in]		fxFwdConv			FX forward market convention
*  @param [in]		fxFwdRates			Forward FX rates
*  @param [in]		spotFxRates			Spot FX rates
*  @return			The curve object handle on the cache
*/
std::string aqCurveObjectCreateBasis( const std::string& aqObjCurveName,
                                       const std::string& curveCollection,
                                       const std::string& staticDataTable,
                                       const std::string& curveIndex,
                                       const SWIG_STRINGMATRIX& curveConv,
                                       const SWIG_STRINGMATRIX& basisConv,
                                       const SWIG_STRINGMATRIX& basisRates,
                                       const SWIG_STRINGMATRIX& fxFwdConv,
                                       const SWIG_STRINGMATRIX& fxFwdRates,
                                       const SWIG_STRINGMATRIX& spotFxRates );

/* @brief			swig interface for aqCurveObjectCreateFXForwards: build a cached FX forwards curve
*  @param [in]		aqObjCurveName		Name for the curve object
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		fxFwdConv			FX forward conventions
*  @return			The curve object handle on the cache
*/
std::string aqCurveObjectCreateFXForwards( const std::string& aqObjCurveName,
                                            const std::string& curveCollection,
                                            const std::string& staticDataTable,
                                            const std::string& curveIndex,
                                            const SWIG_STRINGMATRIX& curveConv,
                                            const SWIG_STRINGMATRIX& fxFwdConv );

/* @brief			swig interface for aqCurveObjectCreateOIS: build a cached OIS curve from raw conventions and rates
*  @param [in]		aqObjCurveName		Name for the curve object
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		oisConv				The OIS curve configuration info
*  @param [in]		oisRates			Constituent OIS instrument rates
*  @param [in]		oisHistoricalRates	Historical OIS fixings
*  @param [in]		liborOisBasisConv	Libor-OIS swap conventions
*  @param [in]		liborOisBasisRates	Libor-OIS basis spreads
*  @param [in]		swapConv			Libor swap conventions
*  @param [in]		swapRates			Libor swap market rates
*  @return			The curve object handle on the cache
*/
std::string aqCurveObjectCreateOIS( const std::string& aqObjCurveName,
                                     const std::string& curveCollection,
                                     const std::string& staticDataTable,
                                     const std::string& curveIndex,
                                     const SWIG_STRINGMATRIX& curveConv,
                                     const SWIG_STRINGMATRIX& oisConv,
                                     const SWIG_STRINGMATRIX& oisRates,
                                     const SWIG_STRINGMATRIX& oisHistoricalRates,
                                     const SWIG_STRINGMATRIX& liborOisBasisConv,
                                     const SWIG_STRINGMATRIX& liborOisBasisRates,
                                     const SWIG_STRINGMATRIX& swapConv,
                                     const SWIG_STRINGMATRIX& swapRates );

/* @brief			swig interface for aqCurveObjectCreateSwap: build a cached Libor swap curve from raw conventions and rates
*  @param [in]		aqObjCurveName		Name for the curve object
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		staticDataTable		Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General yield curve data
*  @param [in]		moneyMarketConv		Money market conventions
*  @param [in]		liborConv			Libor market conventions
*  @param [in]		liborRates			Libor market data
*  @param [in]		swapConv			Swap market conventions
*  @param [in]		swapRates			Swap market data
*  @param [in]		fraConv				FRA market conventions
*  @param [in]		fra3mRates			3M FRA market data
*  @param [in]		fra6mRates			6M FRA market data
*  @param [in]		futureConv			Futures market conventions
*  @param [in]		futureRates			Futures market data
*  @param [in]		convexityAdjConv	Convexity adjustment market conventions
*  @param [in]		convexityAdjRates	Convexity adjustment market data
*  @return			The curve object handle on the cache
*/
std::string aqCurveObjectCreateSwap( const std::string& aqObjCurveName,
                                      const std::string& curveCollection,
                                      const std::string& staticDataTable,
                                      const std::string& curveIndex,
                                      const SWIG_STRINGMATRIX& curveConv,
                                      const SWIG_STRINGMATRIX& moneyMarketConv,
                                      const SWIG_STRINGMATRIX& liborConv,
                                      const SWIG_STRINGMATRIX& liborRates,
                                      const SWIG_STRINGMATRIX& swapConv,
                                      const SWIG_STRINGMATRIX& swapRates,
                                      const SWIG_STRINGMATRIX& fraConv,
                                      const SWIG_STRINGMATRIX& fra3mRates,
                                      const SWIG_STRINGMATRIX& fra6mRates,
                                      const SWIG_STRINGMATRIX& futureConv,
                                      const SWIG_STRINGMATRIX& futureRates,
                                      const SWIG_STRINGMATRIX& convexityAdjConv,
                                      const SWIG_STRINGMATRIX& convexityAdjRates );

/* -------------------------------------------------------------------------
 *  Curve object discount factors / forward rates (handle-based)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectDiscountFactors
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		paymentDates	Column of payment dates
*  @return			Discount factors, aligned with paymentDates
*/
std::vector<double> aqCurveObjectDiscountFactors( const std::string& aqObjCurveName, const std::vector<std::string>& paymentDates );

/* @brief			swig interface for aqCurveObjectDiscountFactorsForwardStarting
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		fromDates		Column of forward-start dates
*  @param [in]		toDates			Column of end dates, aligned with fromDates
*  @return			Forward-starting discount factors
*/
std::vector<double> aqCurveObjectDiscountFactorsForwardStarting( const std::string& aqObjCurveName,
                                                                   const std::vector<std::string>& fromDates,
                                                                   const std::vector<std::string>& toDates );

/* @brief			swig interface for aqCurveObjectDiscountFactorsForwardStartingFromTenors
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		fromDates		Column of forward-start dates
*  @param [in]		tenors			Column of tenors forward from fromDates, aligned
*  @param [in]		businessDayAdj	Business day adjustment for the tenor roll. Default NO_CHANGE
*  @param [in]		calendar		Holiday centre(s) for the tenor roll
*  @return			Forward-starting discount factors
*/
std::vector<double> aqCurveObjectDiscountFactorsForwardStartingFromTenors( const std::string& aqObjCurveName,
                                                                             const std::vector<std::string>& fromDates,
                                                                             const std::vector<std::string>& tenors,
                                                                             const std::string& businessDayAdj,
                                                                             const std::string& calendar );

/* @brief			swig interface for aqCurveObjectDiscountFactorsForwardStartingFromYearFractions
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		fromDates		Column of forward-start dates
*  @param [in]		yearFractions	Column of year fractions forward from fromDates, aligned
*  @param [in]		dayCount		Day count convention used to generate the year fraction
*  @return			Forward-starting discount factors
*/
std::vector<double> aqCurveObjectDiscountFactorsForwardStartingFromYearFractions( const std::string& aqObjCurveName,
                                                                                    const std::vector<std::string>& fromDates,
                                                                                    const std::vector<double>& yearFractions,
                                                                                    const std::string& dayCount );

/* @brief			swig interface for aqCurveObjectDiscountFactorsFromTenors
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		tenors			Column of tenors, e.g. 3M, 5Y
*  @param [in]		businessDayAdj	Business day adjustment for the tenor roll. Default NO_CHANGE
*  @param [in]		calendar		Holiday centre(s) for the tenor roll
*  @return			Discount factors
*/
std::vector<double> aqCurveObjectDiscountFactorsFromTenors( const std::string& aqObjCurveName,
                                                              const std::vector<std::string>& tenors,
                                                              const std::string& businessDayAdj,
                                                              const std::string& calendar );

/* @brief			swig interface for aqCurveObjectDiscountFactorsFromYearFractions
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		yearFractions	Column of year fractions
*  @param [in]		dayCount		Day count convention used to generate the year fraction
*  @return			Discount factors
*/
std::vector<double> aqCurveObjectDiscountFactorsFromYearFractions( const std::string& aqObjCurveName,
                                                                     const std::vector<double>& yearFractions,
                                                                     const std::string& dayCount );

/* @brief			swig interface for aqCurveObjectDiscountFactorsTable: a discount-factor table across
*					one or more curve indices in a curve collection, on a generated date schedule.
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndices		Column of curve indices to include
*  @param [in]		startDate			Table start date. If empty, the curve collection's as-of date is used
*  @param [in]		maturity			Table end tenor/date, e.g. 10Y
*  @param [in]		businessDayAdjust	Business day adjustment, e.g. MODFOLLOWING
*  @param [in]		calendar			Holiday centre(s)
*  @param [in]		rollConvention		Roll convention, e.g. Normal, IMM, EOM
*  @param [in]		frequency			Table row frequency, e.g. MONTHLY
*  @return			A string matrix: first column is the payment date, one further column per curve index
*/
SWIG_STRINGMATRIX aqCurveObjectDiscountFactorsTable( const std::string& curveCollection,
                                                       const std::vector<std::string>& curveIndices,
                                                       const std::string& startDate,
                                                       const std::string& maturity,
                                                       const std::string& businessDayAdjust,
                                                       const std::string& calendar,
                                                       const std::string& rollConvention,
                                                       const std::string& frequency );

/* @brief			swig interface for aqCurveObjectForwardRates: forward rates using the curve's own frequency
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		fixingDates		Column of fixing dates
*  @return			Forward rates, aligned with fixingDates
*/
std::vector<double> aqCurveObjectForwardRates( const std::string& aqObjCurveName, const std::vector<std::string>& fixingDates );

/* @brief			swig interface for aqCurveObjectForwardRatesFromForwardDates
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		fromDates		Column of forward-start dates
*  @param [in]		toDates			Column of forward-end dates, aligned with fromDates
*  @return			Forward rates
*/
std::vector<double> aqCurveObjectForwardRatesFromForwardDates( const std::string& aqObjCurveName,
                                                                  const std::vector<std::string>& fromDates,
                                                                  const std::vector<std::string>& toDates );

/* @brief			swig interface for aqCurveObjectForwardRatesFromYearFraction
*  @param [in]		aqObjCurveName	A curve handle
*  @param [in]		fromDates		Column of forward-start dates
*  @param [in]		yearFraction	Forward period length, in years, common to every date
*  @param [in]		dayCount		Day count convention. Default ACT/365
*  @return			Forward rates
*/
std::vector<double> aqCurveObjectForwardRatesFromYearFraction( const std::string& aqObjCurveName,
                                                                  const std::vector<std::string>& fromDates,
                                                                  double yearFraction,
                                                                  const std::string& dayCount );

/* @brief			swig interface for aqCurveObjectForwardRatesTable: a forward-rate table across one or more
*					curve indices in a curve collection, on a generated date schedule.
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndices		Column of curve indices to include
*  @param [in]		startDate			Table start date. If empty, the curve collection's as-of date is used
*  @param [in]		maturity			Table end tenor/date, e.g. 10Y
*  @param [in]		businessDayAdjust	Business day adjustment, e.g. MODFOLLOWING
*  @param [in]		calendar			Holiday centre(s)
*  @param [in]		rollConvention		Roll convention, e.g. Normal, IMM, EOM
*  @param [in]		frequency			Table row frequency, e.g. MONTHLY
*  @param [in]		fwdInterps			Optional. A list of forward-interpolation flags, one per curve index
*  @return			A string matrix: first column is the fixing date, one further column per curve index
*/
SWIG_STRINGMATRIX aqCurveObjectForwardRatesTable( const std::string& curveCollection,
                                                    const std::vector<std::string>& curveIndices,
                                                    const std::string& startDate,
                                                    const std::string& maturity,
                                                    const std::string& businessDayAdjust,
                                                    const std::string& calendar,
                                                    const std::string& rollConvention,
                                                    const std::string& frequency,
                                                    const std::vector<std::string>& fwdInterps );

/* -------------------------------------------------------------------------
 *  Curve object / market data bumping
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectBumpAll: bump every instrument on a cached curve object
*  @param [in]		objectName						A curve handle
*  @param [in]		bumpSize						Bump size to apply
*  @param [in]		onlyBumpOutrightInstruments		Optional. Default TRUE. Skip basis/spread instruments
*  @return			A status string
*/
std::string aqCurveObjectBumpAll( const std::string& objectName, double bumpSize, bool onlyBumpOutrightInstruments );

/* @brief			swig interface for aqCurveObjectBumpInstrument: bump one instrument type on a cached curve object
*  @param [in]		objectName				A curve handle
*  @param [in]		marketDataType			Which instrument type to bump
*  @param [in]		bumpSize				Bump size to apply
*  @param [in]		clearExistingBumps		Optional. Default FALSE. Clear prior bumps first
*  @return			A status string
*/
std::string aqCurveObjectBumpInstrument( const std::string& objectName, const std::string& marketDataType, double bumpSize, bool clearExistingBumps );

/* @brief			swig interface for aqCurveMarketDataBumpAll: bump every instrument in stored curve market data
*  @param [in]		objectName						A curve-market-data handle
*  @param [in]		bumpSize						Bump size to apply
*  @param [in]		onlyBumpOutrightInstruments		Optional. Default TRUE. Skip basis/spread instruments
*  @return			A status string
*/
std::string aqCurveMarketDataBumpAll( const std::string& objectName, double bumpSize, bool onlyBumpOutrightInstruments );

/* @brief			swig interface for aqCurveMarketDataBumpClear: clear every bump applied to stored curve market data
*  @param [in]		objectName		A curve-market-data handle
*  @return			A status string
*/
std::string aqCurveMarketDataBumpClear( const std::string& objectName );

/* @brief			swig interface for aqCurveMarketDataBumpInstrument: bump one instrument type in stored curve market data
*  @param [in]		objectName				A curve-market-data handle
*  @param [in]		marketDataType			Which instrument type to bump
*  @param [in]		bumpSize				Bump size to apply
*  @param [in]		clearExistingBumps		Optional. Default FALSE. Clear prior bumps first
*  @return			A status string
*/
std::string aqCurveMarketDataBumpInstrument( const std::string& objectName, const std::string& marketDataType, double bumpSize, bool clearExistingBumps );

/* @brief			swig interface for aqCurveMarketDataColumn: one column of curve market data
*  @param [in]		curveObjectName		A curve handle
*  @param [in]		marketDataKey		The data block to read from
*  @param [in]		columnNumber		Which column to return
*  @return			The column, as a single-column string matrix
*/
SWIG_STRINGMATRIX aqCurveMarketDataColumn( const std::string& curveObjectName, const std::string& marketDataKey, int columnNumber );

/* -------------------------------------------------------------------------
 *  Legacy stateless curveCollection+curveIndex surface
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveDelete: remove one curve index from a curve collection
*  @param [in]		curveCollection		Name of curve collection
*  @param [in]		curveIndex			Name of the curve to be removed from the object pool
*  @return			A status string
*/
std::string aqCurveDelete( const std::string& curveCollection, const std::string& curveIndex );

/* @brief			swig interface for aqCurveDiscountFactorsDisplay: every discount factor stored on a curve
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @return			A Term/PaymentDate/DiscountFactor string matrix
*/
SWIG_STRINGMATRIX aqCurveDiscountFactorsDisplay( const std::string& curveCollection, const std::string& curveIndex );

/* @brief			swig interface for aqCurveDiscountFactorsOverride: override a curve's discount factors directly
*  @param [in]		curveCollection				The curve collection
*  @param [in]		curveIndex					The curve index
*  @param [in]		paymentDates				Column of payment dates
*  @param [in]		discountFactors				Column of new discount factors, aligned with paymentDates
*  @param [in]		setCorrespondingForwards	Optional. Default TRUE. Sets STD-curve forwards to zero if FALSE.
*												Note: solving the corresponding forwards can fail to converge for extreme data.
*  @return			A status string
*/
std::string aqCurveDiscountFactorsOverride( const std::string& curveCollection,
                                             const std::string& curveIndex,
                                             const std::vector<std::string>& paymentDates,
                                             const std::vector<double>& discountFactors,
                                             bool setCorrespondingForwards );

/* @brief			swig interface for aqCurveDiscountFactorsSetToOne: set every discount factor on a curve to one
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @return			A status string
*/
std::string aqCurveDiscountFactorsSetToOne( const std::string& curveCollection, const std::string& curveIndex );

/* @brief			swig interface for aqCurveForwardRatesOverride: override a curve's forward rates by setting
*					equivalent discount factors
*  @param [in]		curveCollection							The curve collection
*  @param [in]		curveIndex								The curve index
*  @param [in]		fixingDates								Column of fixing dates
*  @param [in]		forwardRates							Column of new forward rates, aligned with fixingDates
*  @param [in]		setCorrespondingDiscountFactors			Optional. Default TRUE. Also sets STD-curve
*															discount factors to 1.0 if FALSE
*  @return			A status string
*/
std::string aqCurveForwardRatesOverride( const std::string& curveCollection,
                                          const std::string& curveIndex,
                                          const std::vector<std::string>& fixingDates,
                                          const std::vector<double>& forwardRates,
                                          bool setCorrespondingDiscountFactors );

/* @brief			swig interface for aqCurveForwardRatesFromForwardDatesFromObject: as aqCurveForwardRatesFromForwardDates,
*					but the curve identifier may be a curve collection OR a curve handle
*  @param [in]		fromDates					Column of forward-start dates
*  @param [in]		toDates						Column of forward-end dates, aligned with fromDates
*  @param [in]		curveCollectionOrHandle		The curve collection or a curve handle
*  @param [in]		curveIndex					The curve index
*  @param [in]		fwdInter					Optional. Forward-interpolation override; default uses the curve's own setting
*  @param [in]		businessDayAdjust			Optional. Default MODFOLLOWING
*  @return			Forward rates
*/
std::vector<double> aqCurveForwardRatesFromForwardDatesFromObject( const std::vector<std::string>& fromDates,
                                                                     const std::vector<std::string>& toDates,
                                                                     const std::string& curveCollectionOrHandle,
                                                                     const std::string& curveIndex,
                                                                     const std::string& fwdInter,
                                                                     const std::string& businessDayAdjust );

/* -------------------------------------------------------------------------
 *  Curve Jacobian risk display
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveObjectEngineJacobianDisplay: display the multi-curve yield curve
*					engine's Jacobian matrix, or its row/column labels
*  @param [in]		curveEngineObject		A curve-engine handle (from aqCurveObjectEngineCalibrate)
*  @param [in]		curveCollection			The curve collection
*  @param [in]		displayLabels			Optional. Default TRUE. Display the label matrix instead of the values
*  @param [in]		displayInverseMatrix	Optional. Default FALSE. Display the inverse Jacobian
*  @return			The requested matrix (values or labels), as a string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectEngineJacobianDisplay( const std::string& curveEngineObject,
                                                        const std::string& curveCollection,
                                                        bool displayLabels,
                                                        bool displayInverseMatrix );

/* @brief			swig interface for aqCurveObjectJacobianDisplay: display one yield curve's Jacobian matrix
*  @param [in]		curveCollection			The curve collection
*  @param [in]		curveName				The curve index/name
*  @param [in]		displayInverseMatrix	Optional. Default FALSE. Display the inverse Jacobian
*  @return			The Jacobian matrix, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveObjectJacobianDisplay( const std::string& curveCollection, const std::string& curveName, bool displayInverseMatrix );

/* -------------------------------------------------------------------------
 *  Curve results / Jacobian risk store
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveResultsEnable: enable or disable the curve results (Jacobian risk) store
*  @param [in]		enable		TRUE to enable, FALSE to disable
*  @return			A status string
*/
std::string aqCurveResultsEnable( bool enable );

/* @brief			swig interface for aqCurveResultsIsEnabled: whether the curve results store is currently enabled
*  @return			A status string
*/
std::string aqCurveResultsIsEnabled();

// forwardAdjustments (unlike the three *LVB params) has no fixed column count; see the matching note
// in aqToolData.h -- excluded from the R binding rather than guessed at.
#if (!defined(SWIG_R)) && (!defined(SWIGR))
/* @brief			swig interface for aqCurveResultsDiscountFactorsUpdate: update stored discount factors for a curve results object
*  @param [in]		curveLVB			Curve identity, as a label/value block
*  @param [in]		parameterLVB		Calibration parameters, as a label/value block
*  @param [in]		discountFactorLVB	Discount factors to store, as a label/value block
*  @param [in]		forwardAdjustments	Optional. Forward-rate adjustments
*  @return			A status string
*/
std::string aqCurveResultsDiscountFactorsUpdate( const SWIG_STRINGMATRIX& curveLVB,
                                                   const SWIG_STRINGMATRIX& parameterLVB,
                                                   const SWIG_STRINGMATRIX& discountFactorLVB,
                                                   const SWIG_STRINGMATRIX& forwardAdjustments );
#endif

/* @brief			swig interface for aqCurveResultsDiscountFactorsDisplay: discount factors from a curve results
*					object at a column of payment dates
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @param [in]		paymentDates		Column of payment dates
*  @return			Discount factors, aligned with paymentDates
*/
std::vector<double> aqCurveResultsDiscountFactorsDisplay( const std::string& curveCollection,
                                                             const std::string& curveIndex,
                                                             const std::vector<std::string>& paymentDates );

/* @brief			swig interface for aqCurveResultsDelete: delete one curve results object
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @return			A status string
*/
std::string aqCurveResultsDelete( const std::string& curveCollection, const std::string& curveIndex );

/* @brief			swig interface for aqCurveResultsDeleteAll: delete every curve results object
*  @return			A status string
*/
std::string aqCurveResultsDeleteAll();

/* @brief			swig interface for aqCurveResultsForwardRatesDisplay: forward rates from a curve results
*					object at a column of fixing dates
*  @param [in]		curveCollection			The curve collection
*  @param [in]		curveIndex				The curve index
*  @param [in]		fixingDates				Column of fixing dates
*  @param [in]		isFwdInter				Optional. Default FALSE
*  @param [in]		fixingBusinessDayAdj	Optional. Default NONE. Business day adjustment for the fixing dates
*  @param [in]		fixingCalendar			Optional. Holiday centre(s) for the fixing dates
*  @return			Forward rates, aligned with fixingDates
*/
std::vector<double> aqCurveResultsForwardRatesDisplay( const std::string& curveCollection,
                                                          const std::string& curveIndex,
                                                          const std::vector<std::string>& fixingDates,
                                                          bool isFwdInter,
                                                          const std::string& fixingBusinessDayAdj,
                                                          const std::string& fixingCalendar );

/* @brief			swig interface for aqCurveResultsJacobianUpdate: update the stored Jacobian for a curve results object
*  @param [in]		curveLVB						Curve identity, as a label/value block
*  @param [in]		discountFactorParameterLVB		Discount factor parameters, as a label/value block
*  @param [in]		discountFactors					Discount factors, as a label/value block
*  @param [in]		jacobianParameterLVB			Jacobian parameters, as a label/value block
*  @param [in]		outrightInstruments				One flag per calibration instrument
*  @param [in]		marketDataShiftSizeInPercent	Market-data shift sizes used to build the Jacobian
*  @param [in]		jacobianMatrix					The Jacobian matrix values
*  @return			A status string
*/
std::string aqCurveResultsJacobianUpdate( const SWIG_STRINGMATRIX& curveLVB,
                                            const SWIG_STRINGMATRIX& discountFactorParameterLVB,
                                            const SWIG_STRINGMATRIX& discountFactors,
                                            const SWIG_STRINGMATRIX& jacobianParameterLVB,
                                            const std::vector<bool>& outrightInstruments,
                                            const std::vector<double>& marketDataShiftSizeInPercent,
                                            const SWIG_STRINGMATRIX& jacobianMatrix );

/* @brief			swig interface for aqCurveResultsJacobianDisplay: display the Jacobian matrix stored for a curve results object
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @param [in]		riskType			The risk type to display
*  @return			The Jacobian matrix, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianDisplay( const std::string& curveCollection, const std::string& curveIndex, const std::string& riskType );

/* @brief			swig interface for aqCurveResultsJacobianDiscountFactorDelta: the Jacobian discount-factor delta
*					for a curve results object
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @return			The Jacobian discount-factor delta, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianDiscountFactorDelta( const std::string& curveCollection, const std::string& curveIndex );

/* @brief			swig interface for aqCurveResultsJacobianRiskTotals: Jacobian risk totals for a curve results object
*  @param [in]		curveCollection				The curve collection
*  @param [in]		curveIndex					The curve index
*  @param [in]		riskType					The risk type to total
*  @param [in]		useOutrightInstrumentsOnly	Optional. Default TRUE
*  @return			The risk totals, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianRiskTotals( const std::string& curveCollection,
                                                      const std::string& curveIndex,
                                                      const std::string& riskType,
                                                      bool useOutrightInstrumentsOnly );

/* @brief			swig interface for aqCurveResultsJacobianImplyNewDiscountFactors: discount factors implied by the
*					Jacobian after a market-data shift, for a curve results object
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @return			The implied discount factors, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveResultsJacobianImplyNewDiscountFactors( const std::string& curveCollection, const std::string& curveIndex );

/* -------------------------------------------------------------------------
 *  Curve groups
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveGroupCreate: create a named group of curve handles
*  @param [in]		groupName		Name for the curve group
*  @param [in]		curveHandles	Column of curve handles to include
*  @return			A status string
*/
std::string aqCurveGroupCreate( const std::string& groupName, const std::vector<std::string>& curveHandles );

/* @brief			swig interface for aqCurveGroupCollectionName: the curve collection name for a given curve group
*  @param [in]		groupName		A curve group name
*  @return			The curve collection name
*/
std::string aqCurveGroupCollectionName( const std::string& groupName );

/* -------------------------------------------------------------------------
 *  Curve terms / dates
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveTermsToDates: payment dates for a column of term year fractions,
*					from a curve collection's as-of date
*  @param [in]		curveCollection		The curve collection
*  @param [in]		terms				Column of term year fractions
*  @return			Payment dates, aligned with terms
*/
std::vector<std::string> aqCurveTermsToDates( const std::string& curveCollection, const std::vector<double>& terms );

/* @brief			swig interface for aqCurveDatesToTerms: term year fractions for a column of payment dates,
*					from a curve collection's as-of date
*  @param [in]		curveCollection		The curve collection
*  @param [in]		paymentDates		Column of payment dates
*  @return			Term year fractions, aligned with paymentDates
*/
std::vector<double> aqCurveDatesToTerms( const std::string& curveCollection, const std::vector<std::string>& paymentDates );

/* -------------------------------------------------------------------------
 *  Curve compound rate with fixing table
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveCompoundRateWithFixingTable: as aqCurveCompoundRate, applying a
*					spread over a cached fixing table's resets
*  @param [in]		startDates			Column of accrual start dates
*  @param [in]		endDates			Column of accrual end dates, aligned with startDates
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		forecastCurveIndex	Forecasting curve index
*  @param [in]		frequency			Compounding frequency
*  @param [in]		spread				Spread in basis points
*  @param [in]		stubType			None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE)
*  @param [in]		rollDayInput		Roll day convention, e.g. ENDDATE
*  @param [in]		calendar			Holiday centre(s)
*  @param [in]		businessDayAdj		Business day adjustment, e.g. MODFOLLOWING
*  @param [in]		dayCount			Day count convention
*  @param [in]		interpolation		Interpolation method
*  @param [in]		compoundType		NORMAL, FLAT, SIMPLE or AVERAGE
*  @param [in]		firstStubDate		End date of the front stub period
*  @param [in]		lastStubDate		Start date of the end stub period
*  @param [in]		fixingTableName		A fixing table handle for the spread's reset basis
*  @param [in]		annualized			Optional. Default TRUE. Return an annualized rate
*  @return			Interest rates calculated based on startDates, endDates and compound type
*/
std::vector<double> aqCurveCompoundRateWithFixingTable( const std::vector<std::string>& startDates,
                                                           const std::vector<std::string>& endDates,
                                                           const std::string& curveCollection,
                                                           const std::string& forecastCurveIndex,
                                                           const std::string& frequency,
                                                           double spread,
                                                           const std::string& stubType,
                                                           const std::string& rollDayInput,
                                                           const std::string& calendar,
                                                           const std::string& businessDayAdj,
                                                           const std::string& dayCount,
                                                           const std::string& interpolation,
                                                           const std::string& compoundType,
                                                           const std::string& firstStubDate,
                                                           const std::string& lastStubDate,
                                                           const std::string& fixingTableName,
                                                           bool annualized );

/* @brief			swig interface for aqCurveCompoundRateWithFixingTable (single-date form)
*  @param [in]		startDate			Accrual start date
*  @param [in]		endDate				Accrual end date
*  @param [in]		curveCollection		ID of the curve set
*  @param [in]		forecastCurveIndex	Forecasting curve index
*  @param [in]		frequency			Compounding frequency
*  @param [in]		spread				Spread in basis points
*  @param [in]		stubType			None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE)
*  @param [in]		rollDayInput		Roll day convention, e.g. ENDDATE
*  @param [in]		calendar			Holiday centre(s)
*  @param [in]		businessDayAdj		Business day adjustment, e.g. MODFOLLOWING
*  @param [in]		dayCount			Day count convention
*  @param [in]		interpolation		Interpolation method
*  @param [in]		compoundType		NORMAL, FLAT, SIMPLE or AVERAGE
*  @param [in]		firstStubDate		End date of the front stub period
*  @param [in]		lastStubDate		Start date of the end stub period
*  @param [in]		fixingTableName		A fixing table handle for the spread's reset basis
*  @param [in]		annualized			Optional. Default TRUE. Return an annualized rate
*  @return			Compounding rate calculated based on startDate, endDate and compound type
*/
double aqCurveCompoundRateWithFixingTable( const std::string& startDate,
                                             const std::string& endDate,
                                             const std::string& curveCollection,
                                             const std::string& forecastCurveIndex,
                                             const std::string& frequency,
                                             double spread,
                                             const std::string& stubType,
                                             const std::string& rollDayInput,
                                             const std::string& calendar,
                                             const std::string& businessDayAdj,
                                             const std::string& dayCount,
                                             const std::string& interpolation,
                                             const std::string& compoundType,
                                             const std::string& firstStubDate,
                                             const std::string& lastStubDate,
                                             const std::string& fixingTableName,
                                             bool annualized );

/* -------------------------------------------------------------------------
 *  Curve one-shot calibration - cheapest-to-deliver collateral curve
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveCalibrateCTD: one-shot calibration of a cheapest-to-deliver
*					collateral curve
*  @param [in]		curveCollection		The curve collection ID
*  @param [in]		curveName			Name of the curve constructed by this method
*  @param [in]		curveIndex			Equivalent names of the curve being built
*  @param [in]		curveConv			General curve properties: as-of date, ccy, interpolation, etc
*  @param [in]		collateralCurves	The group of collateral curves the CTD curve is constructed from
*  @return			A status string
*/
std::string aqCurveCalibrateCTD( const std::string& curveCollection,
                                   const std::string& curveName,
                                   const std::string& curveIndex,
                                   const SWIG_STRINGMATRIX& curveConv,
                                   const std::vector<std::string>& collateralCurves );

/* -------------------------------------------------------------------------
 *  Curve floating index frequency / EuroDollar convexity adjustment
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveFrequency: the floating index frequency of a curve collection/index
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @return			The floating index frequency, e.g. 3M
*/
std::string aqCurveFrequency( const std::string& curveCollection, const std::string& curveIndex );

/* @brief			swig interface for aqCurveEuroDollarConvexityAdjustment: the EuroDollar futures convexity
*					adjustment under a Hull-White 1F model
*  @param [in]		curveAsOfDate		The yield curve as-of / valuation date
*  @param [in]		futuresStartDate	The futures start date
*  @param [in]		futuresEndDate		The futures end date
*  @param [in]		meanReversion		The Hull-White 1F mean reversion parameter
*  @param [in]		volatility			The Hull-White 1F volatility parameter
*  @return			The EuroDollar futures convexity adjustment
*/
double aqCurveEuroDollarConvexityAdjustment( const std::string& curveAsOfDate,
                                               const std::string& futuresStartDate,
                                               const std::string& futuresEndDate,
                                               double meanReversion,
                                               double volatility );

/* -------------------------------------------------------------------------
 *  Short-rate model checks (Hull-White, Vasicek)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveHullWhiteForwardRates: forward rates implied by a Hull-White 1F
*					short-rate model, for comparison against the curve's own forwards
*  @param [in]		fixingDates		Column of fixing dates
*  @param [in]		curveCollection	The curve collection
*  @param [in]		curveIndex		The curve index
*  @param [in]		sigma			The Hull-White 1F volatility parameter
*  @param [in]		alpha			The Hull-White 1F mean reversion speed parameter
*  @param [in]		rt				Short rate at time t; NaN derives it from the curve
*  @param [in]		valuationDate	Optional. Default the curve's as-of date (blank string uses the default)
*  @return			Forward rates, aligned with fixingDates
*/
std::vector<double> aqCurveHullWhiteForwardRates( const std::vector<std::string>& fixingDates,
                                                     const std::string& curveCollection,
                                                     const std::string& curveIndex,
                                                     double sigma,
                                                     double alpha,
                                                     double rt,
                                                     const std::string& valuationDate );

/* @brief			swig interface for aqCurveVasicekChecking: fits a Vasicek 1F model's initial volatility to
*					target forward rates; a near-zero result confirms the fit
*  @param [in]		fixingDates			Column of fixing dates
*  @param [in]		targetForwardRates	Column of target forward rates to fit to, aligned with fixingDates
*  @param [in]		curveCollection		The curve collection
*  @param [in]		curveIndex			The curve index
*  @param [in]		initialTheta		The Vasicek 1F theta parameter
*  @param [in]		initialSigma		The Vasicek 1F volatility parameter
*  @param [in]		alpha				The Vasicek 1F mean reversion speed parameter
*  @param [in]		rt					Short rate at time t; NaN derives it from the curve
*  @param [in]		valuationDate		Optional. Default the curve's as-of date (blank string uses the default)
*  @param [in]		showColumnHeaders	Optional. Default TRUE
*  @return			The initial volatility fit, should be close to zero
*/
SWIG_STRINGMATRIX aqCurveVasicekChecking( const std::vector<std::string>& fixingDates,
                                            const std::vector<double>& targetForwardRates,
                                            const std::string& curveCollection,
                                            const std::string& curveIndex,
                                            double initialTheta,
                                            double initialSigma,
                                            double alpha,
                                            double rt,
                                            const std::string& valuationDate,
                                            bool showColumnHeaders );

/* @brief			swig interface for aqCurveVasicekForwardRates: forward rates implied by a Vasicek 1F
*					short-rate model, for comparison against the curve's own forwards
*  @param [in]		fixingDates		Column of fixing dates
*  @param [in]		curveCollection	The curve collection
*  @param [in]		curveIndex		The curve index
*  @param [in]		theta			The Vasicek 1F theta parameter
*  @param [in]		sigma			The Vasicek 1F volatility parameter
*  @param [in]		alpha			The Vasicek 1F mean reversion speed parameter
*  @param [in]		rt				Short rate at time t; NaN derives it from the curve
*  @param [in]		valuationDate	Optional. Default the curve's as-of date (blank string uses the default)
*  @return			Forward rates, aligned with fixingDates
*/
std::vector<double> aqCurveVasicekForwardRates( const std::vector<std::string>& fixingDates,
                                                   const std::string& curveCollection,
                                                   const std::string& curveIndex,
                                                   double theta,
                                                   double sigma,
                                                   double alpha,
                                                   double rt,
                                                   const std::string& valuationDate );

/* -------------------------------------------------------------------------
 *  Curve generator (a named sub-object - conventions)
 * ---------------------------------------------------------------------- */

/* @brief			swig interface for aqCurveGeneratorCreate: create and store a curve generator from one or
*					two named data blocks
*  @param [in]		objectName	Name for the curve-generator object
*  @param [in]		key1		Name of the first data block
*  @param [in]		value1		First data block
*  @param [in]		key2		Optional. Name of the second data block
*  @param [in]		value2		Optional. Second data block
*  @return			The curve-generator handle on the cache
*/
std::string aqCurveGeneratorCreate( const std::string& objectName,
                                      const std::string& key1, const SWIG_STRINGMATRIX& value1,
                                      const std::string& key2, const SWIG_STRINGMATRIX& value2 );

/* @brief			swig interface for aqCurveGeneratorDisplay: display a stored curve generator's configuration
*					block as a matrix
*  @param [in]		objectName		A curve-generator handle
*  @param [in]		propertyName	The configuration block to display
*  @return			The requested configuration block, as a string matrix
*/
SWIG_STRINGMATRIX aqCurveGeneratorDisplay( const std::string& objectName, const std::string& propertyName );

/* @brief			swig interface for aqCurveGeneratorModify: create a new curve generator by overriding values
*					on an existing one
*  @param [in]		newObjectName	Name for the new curve-generator object
*  @param [in]		baseObjectName	The existing curve-generator handle to copy from
*  @param [in]		modifiedValues	The overrides as a label/value block
*  @return			The new curve-generator handle on the cache
*/
std::string aqCurveGeneratorModify( const std::string& newObjectName,
                                      const std::string& baseObjectName,
                                      const SWIG_STRINGMATRIX& modifiedValues );
