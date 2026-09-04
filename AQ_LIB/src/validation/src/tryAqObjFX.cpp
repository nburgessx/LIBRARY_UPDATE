#include "tryAqObjFX.h"
#include "FXCurve.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CoreEnumerations.h"
#include "RecordMacros.h"
#include "AQObjUtilities.h"
#include "ObjectUtilities.h"
#include "FXCurveUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{

    /* @brief validation interface for aqObjFXCurveCreate, creating a FxCurve using a CurveGenerator object and a CurveMarketData object
	 * @param [in] objectName                The objectName of the FxCurve
     * @param [in] aqObjCurveGeneratorName     The name of the AQObjCurveGenerator object to use
	 * @param [in] aqObjCurveMarketDataName    The name of the AQObjCurveMarketData object to use
	 * @param [out]                          The created FxCurve objectName
	 */
    std::string tryAqObjFXCurveCreate(	const std::string& objectName,
										const std::string& aqObjCurveGeneratorName,
										const std::string& aqObjCurveMarketDataName )
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, aqObjCurveGeneratorName, aqObjCurveMarketDataName );

		// Perform initial basic sanity checks
		if ( aqObjCurveGeneratorName.size() == 0 )
		{
				throw AQLCoreInvalidData(	( "#Error: Missing aqObjCurveGenerator name" ), __FILE__, __LINE__ );
		}
		if ( aqObjCurveMarketDataName.size() == 0 )
		{
				throw AQLCoreInvalidData(	( "#Error: Missing aqObjCurveMarketData name" ), __FILE__, __LINE__ );
		}

		// Attempt to retrieve AQObjCurveMarketData object from the AQObj object cache
		auto aqObjCurveMarketData = etrading::getCurveMarketData( aqObjCurveMarketDataName );
  
		// Attempt to retrieve AQObjCurveGenerator object from the AQObj object cache
		auto aqObjCurveGenerator = etrading::getCurveGenerator( aqObjCurveGeneratorName );
       
		// Check for matching IdentityParams from the CurveGenerator and CurveMarketData
		const LabelValueBlock curvePropertiesLVB = aqObjCurveGenerator->toLabelValueBlock( "CURVEPROPERTIES" );
		const std::string configCurrency         = curvePropertiesLVB.getCompulsoryValue( "Currency" );
		const std::string configCurveType        = curvePropertiesLVB.getCompulsoryValue( "CurveType" );
		
		const LabelValueBlock marketDataPropertiesLVB = aqObjCurveMarketData->toLabelValueBlock( "MARKETDATAPROPERTIES" );
		const std::string marketDataCurrency          = marketDataPropertiesLVB.getCompulsoryValue( "Currency" );
		const std::string marketDataCurveType         = marketDataPropertiesLVB.getCompulsoryValue( "CurveType" );
		
        etrading::CurveTypeEnum curveTypeEnum = etrading::toCurveTypeEnum( configCurveType );

        if ( configCurrency != marketDataCurrency )
		{
			throw AQLCoreInvalidData( ( boost::format("#Error: CurveGenerator currency \"%s\" does not match MarketData Currency \"%s\"" )
                                   % configCurrency % marketDataCurrency ).str().c_str(), __FILE__, __LINE__ );
		}
		if ( configCurveType != marketDataCurveType )
		{
			throw AQLCoreInvalidData( ( boost::format("#Error: CurveGenerator CurveType \"%s\" does not match MarketData CurveType \"%s\"" )
                                   % configCurveType % marketDataCurveType ).str().c_str(), __FILE__, __LINE__ );
		}
		if (curveTypeEnum != etrading::FX_CURVETYPE)
		{
			throw AQLCoreInvalidData( ( boost::format("#Error: CurveGenerator CurveType \"%s\" is not FX" )
                                   % configCurveType ).str().c_str(), __FILE__, __LINE__ );
		}

		// Create the FxCurve object
        etrading::FXCurve fxCurve(objectName, aqObjCurveGenerator, aqObjCurveMarketData);

        // ..  and store in the cache
        etrading::copyToCache<etrading::FXCurve>( fxCurve );

		// Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( objectName);

		VALID_EXCEPTION_END

    }


    /* @brief validation interface for aqFXForwardsFromDiscountCurves
	 * @param [in] objectName           The objectName of the FxCurve
     * @param [in] settleDatesOrTenors  Settlement dates or tenors
     * @param [in] baseCurveCollection  Base currency curve collection
	 * @param [in] termCurveCollection  Term currency curve collection
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
 	 */
    AnyTypeMatrix tryAqObjFXForwardsFromDiscountCurves(const std::string& objectName,
        											const std::vector<std::string>& settleDatesOrTenors, 
													const std::string& baseCurveCollection, 
													const std::string& termCurveCollection,
													const bool showColumnHeaders,
													const std::vector<std::string>& columnList)
    {
		VALID_EXCEPTION_START
	    
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, settleDatesOrTenors, baseCurveCollection, termCurveCollection, showColumnHeaders, columnList);

        auto fxCurve = etrading::getFxCurve(objectName);

        auto ret = etrading::outputFxPrice(fxCurve->getFxForwardsFromDiscountCurves(settleDatesOrTenors, baseCurveCollection, termCurveCollection), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( ret);

		VALID_EXCEPTION_END

    }

	/* @brief validation interface for aqFXForwards
	 * @param [in] objectName           The objectName of the FxCurve
     * @param [in] settleDatesOrTenors  Settlement dates or tenors
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
 	 */
    AnyTypeMatrix tryAqObjFXForwards(const std::string& objectName,
        							const std::vector<std::string>& settleDatesOrTenors, 
									const bool showColumnHeaders,
								    const std::vector<std::string>& columnList)
    {
		VALID_EXCEPTION_START
	    
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, settleDatesOrTenors, showColumnHeaders, columnList);

        auto fxCurve = etrading::getFxCurve(objectName);
        
        auto ret = etrading::outputFxPrice(fxCurve->getFxForwardsFromFxCurve(settleDatesOrTenors), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( ret);

		VALID_EXCEPTION_END

    }


    /* @brief validation interface for aqObjFXSwapFromDiscountCurves
	 * @param [in] objectName						The objectName of the FxCurve
     * @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
     * @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
     * @param [in] baseCurveCollection				Base currency curve collection
	 * @param [in] termCurveCollection				Term currency curve collection
     * @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	 * @param [in] showColumnHeaders				True to show column headers, default to true
	 * @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]									Fx Swap outright rates and points
 	 */
    AnyTypeMatrix tryAqObjFXSwapFromDiscountCurves(const std::string& objectName,
        										const std::vector<std::string>& nearLegSettlementDatesOrTenors, 
                                                const std::vector<std::string>& farLegSettlementDatesOrTenors, 
                                                const std::string& baseCurveCollection, 
                                                const std::string& termCurveCollection,
												const bool outputFarLeg,
												const bool showColumnHeaders,
												const std::vector<std::string>& columnList)
    {
		VALID_EXCEPTION_START
	    
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, baseCurveCollection, termCurveCollection, outputFarLeg, showColumnHeaders, columnList);

        auto fxCurve = etrading::getFxCurve(objectName);
        
        auto ret = etrading::outputFxPrice(fxCurve->getFxSwapFromDiscountCurves(nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, baseCurveCollection, termCurveCollection, outputFarLeg), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( ret);

		VALID_EXCEPTION_END

    }

	/* @brief validation interface for aqObjFXSwap
	 * @param [in] objectName						The objectName of the FxCurve
     * @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
     * @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
     * @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	 * @param [in] showColumnHeaders				True to show column headers, default to true
	 * @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]									Fx Swap outright rates and points
 	 */
    AnyTypeMatrix tryAqObjFXSwap(const std::string& objectName,
        						const std::vector<std::string>& nearLegSettlementDatesOrTenors, 
								const std::vector<std::string>& farLegSettlementDatesOrTenors, 
								const bool outputFarLeg,
								const bool showColumnHeaders,
								const std::vector<std::string>& columnList)
    {
		VALID_EXCEPTION_START
	    
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, outputFarLeg, showColumnHeaders, columnList);

        auto fxCurve = etrading::getFxCurve(objectName);
        
        auto ret = etrading::outputFxPrice(fxCurve->getFxSwapFromFxCurve(nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, outputFarLeg), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( ret);

		VALID_EXCEPTION_END

    }





	/* @brief validation interface for aqObjFXForwardsFromXccyCurveObject
	 * @param [in] xccyCurveObjectName  xccyCurveObjectName
	 * @param [in] settleDatesOrTenors  Settlement dates or tenors
	 * @param [in] swapRateSpread       XccyCurve SwapRate Spread
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
	 */
	AnyTypeMatrix tryAqObjFXForwardsFromXccyCurveObject(const std::string& xccyCurveObjectName,
												const std::vector<std::string>& settleDatesOrTenors,
												const double xccyCurveSwapRateBumpSize,
												const bool showColumnHeaders,
												const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(xccyCurveObjectName, settleDatesOrTenors, xccyCurveSwapRateBumpSize, showColumnHeaders, columnList);

		auto& env = etrading::Environment::defaultEnv();
		auto curveObject = etrading::getSingleCurveObject(xccyCurveObjectName);

		auto curveFxInfo = etrading::populateCurveFxFwdDataProvider(curveObject);

		auto ret = etrading::outputFxPrice(etrading::calculateFxForwardsFromDiscountCurves(settleDatesOrTenors, curveFxInfo, xccyCurveSwapRateBumpSize), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		VALID_EXCEPTION_END

	}

	/* @brief validation interface for aqObjFXForwardsFromXccyCurve
	 * @param [in] baseCurveCollection  baseCurveCollection
	 * @param [in] baseCurveIndex		baseCurveIndex
	 * @param [in] termCurveCollection  termCurveCollection
	 * @param [in] termCurveIndex		termCurveIndex
	 * @param [in] fxSpotRate			fxSpotRate
	 * @param [in] pipSize				pipSize
	 * @param [in] settleDatesOrTenors  Settlement dates or tenors
	 * @param [in] xccyCurveSwapRateBumpSize       XccyCurve SwapRate Bump Size
	 * @param [in] settleDatesOrTenors       settlement dates or tenors
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
	 */
	AnyTypeMatrix tryAqObjFXForwardsFromXccyCurve(const std::string& baseCurveCollection,
												const std::string& baseCurveIndex,
												const std::string& termCurveCollection,
												const std::string& termCurveIndex,
												const double fxSpotRate,
												const double pipSize,
												const double xccyCurveSwapRateBumpSize,
												const std::vector<std::string>& settleDatesOrTenors,
												const bool showColumnHeaders,
												const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(baseCurveCollection, baseCurveIndex, termCurveCollection, termCurveIndex, fxSpotRate, pipSize, xccyCurveSwapRateBumpSize, settleDatesOrTenors, showColumnHeaders, columnList);

		auto curveFxInfo = etrading::populateCurveFxFwdDataProvider(baseCurveCollection, baseCurveIndex, termCurveCollection, termCurveIndex, fxSpotRate, pipSize);

		auto ret = etrading::outputFxPrice(etrading::calculateFxForwardsFromDiscountCurves(settleDatesOrTenors, curveFxInfo, xccyCurveSwapRateBumpSize), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		VALID_EXCEPTION_END

	}

	/* @brief validation interface for aqObjFXSwapFromXccyCurve
	* @param [in] xccyCurveObjectName				xccyCurveObjectName
	* @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
	* @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
	* @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	* @param [in] showColumnHeaders				True to show column headers, default to true
	* @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	* @param [out]									Fx Swap outright rates and points
	*/
	AnyTypeMatrix tryAqObjFXSwapFromXccyCurveObject(const std::string& xccyCurveObjectName,
													const std::vector<std::string>& nearLegSettlementDatesOrTenors,
													const std::vector<std::string>& farLegSettlementDatesOrTenors,
													const bool outputFarLeg,
													const bool showColumnHeaders,
													const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(xccyCurveObjectName, nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, outputFarLeg, showColumnHeaders, columnList);

		auto& env = etrading::Environment::defaultEnv();
		auto curveObject = etrading::getSingleCurveObject(xccyCurveObjectName);

		auto curveFxInfo = etrading::populateCurveFxFwdDataProvider(curveObject);

		auto ret = etrading::outputFxPrice(etrading::calculateFxSwapFromDiscountCurves(nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, curveFxInfo, outputFarLeg), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		VALID_EXCEPTION_END

	}

	
	/* @brief validation interface for aqObjFXSwapFromXccyCurve
	 * @param [in] baseCurveCollection  baseCurveCollection
	 * @param [in] baseCurveIndex		baseCurveIndex
	 * @param [in] termCurveCollection  termCurveCollection
	 * @param [in] termCurveIndex		termCurveIndex
	 * @param [in] fxSpotRate			fxSpotRate
	 * @param [in] pipSize				pipSize
	* @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
	* @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
	* @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	* @param [in] showColumnHeaders				True to show column headers, default to true
	* @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	* @param [out]									Fx Swap outright rates and points
	*/
	AnyTypeMatrix tryAqObjFXSwapFromXccyCurve(const std::string& baseCurveCollection,
											const std::string& baseCurveIndex,
											const std::string& termCurveCollection,
											const std::string& termCurveIndex,
											const double fxSpotRate,
											const double pipSize,
											const std::vector<std::string>& nearLegSettlementDatesOrTenors,
											const std::vector<std::string>& farLegSettlementDatesOrTenors,
											const bool outputFarLeg,
											const bool showColumnHeaders,
											const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(baseCurveCollection, baseCurveIndex, termCurveCollection, termCurveIndex, fxSpotRate, pipSize, nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, outputFarLeg, showColumnHeaders, columnList);

		auto curveFxInfo = etrading::populateCurveFxFwdDataProvider(baseCurveCollection, baseCurveIndex, termCurveCollection, termCurveIndex, fxSpotRate, pipSize);

		auto ret = etrading::outputFxPrice(etrading::calculateFxSwapFromDiscountCurves(nearLegSettlementDatesOrTenors, farLegSettlementDatesOrTenors, curveFxInfo, outputFarLeg), showColumnHeaders, etrading::toFXPriceEnumSet(columnList));

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		VALID_EXCEPTION_END

	}

	/*	@brief		validation interface for the aqObjFXSpotToAsOfDate method
	*	@param [in]	fxSpot					FX rate on FX Spot Date
	*	@param [in]	fxSpotDate				FX Spot Date
	*	@param [in]	baseCurveCollection		baseCurveCollection
	*	@param [in]	baseDiscountCurveIndex	baseDiscountCurveIndex
	*	@param [in]	termCurveCollection		termCurveCollection
	*	@param [in]	termDiscountCurveIndex	termDiscountCurveIndex
	*	@return		FxRate on curve as of date
	*/
	double tryAqObjFXSpotToAsOfDate(const double fxSpot, const AQLDate& fxSpotDate, const std::string& baseCurveCollection, const std::string& baseDiscountCurveIndex, const std::string& termCurveCollection, const std::string& termDiscountCurveIndex)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(fxSpot, fxSpotDate, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex);

		double ret = etrading::fxRateFromSpotToAsOfDate(fxSpot, fxSpotDate, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		VALID_EXCEPTION_END
	}

	/*	@brief		validation interface for the aqObjFXAsOfDateToSpot method
	*	@param [in]	fxAsOfDateRate			FX rate on As of Date
	*	@param [in]	fxSpotDate				FX Spot Date
	*	@param [in]	baseCurveCollection		baseCurveCollection
	*	@param [in]	baseDiscountCurveIndex	baseDiscountCurveIndex
	*	@param [in]	termCurveCollection		termCurveCollection
	*	@param [in]	termDiscountCurveIndex	termDiscountCurveIndex
	*	@return		FxRate on curve as of date
	*/
	double tryAqObjFXAsOfDateToSpot(const double fxAsOfDateRate, const AQLDate& fxSpotDate, const std::string& baseCurveCollection, const std::string& baseDiscountCurveIndex, const std::string& termCurveCollection, const std::string& termDiscountCurveIndex)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(fxAsOfDateRate, fxSpotDate, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex);

		double ret = etrading::fxRateFromAsOfDateToSpot(fxAsOfDateRate, fxSpotDate, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		VALID_EXCEPTION_END
	}



       

}