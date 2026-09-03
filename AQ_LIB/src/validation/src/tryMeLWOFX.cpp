#include "tryMeLWOFX.h"
#include "FXCurve.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CoreEnumerations.h"
#include "RecordMacros.h"
#include "LWOUtilities.h"
#include "ObjectUtilities.h"
#include "FXCurveUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{

    /* @brief validation interface for meLWOFXCurveCreate, creating a FxCurve using a CurveGenerator object and a CurveMarketData object
	 * @param [in] objectName                The objectName of the FxCurve
     * @param [in] lwoCurveGeneratorName     The name of the LWOCurveGenerator object to use
	 * @param [in] lwoCurveMarketDataName    The name of the LWOCurveMarketData object to use
	 * @param [out]                          The created FxCurve objectName
	 */
    std::string tryMeLWOFXCurveCreate(	const std::string& objectName,
										const std::string& lwoCurveGeneratorName,
										const std::string& lwoCurveMarketDataName )
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, lwoCurveGeneratorName, lwoCurveMarketDataName );

		// Perform initial basic sanity checks
		if ( lwoCurveGeneratorName.size() == 0 )
		{
				throw LACoreInvalidData(	( "#Error: Missing lwoCurveGenerator name" ), __FILE__, __LINE__ );
		}
		if ( lwoCurveMarketDataName.size() == 0 )
		{
				throw LACoreInvalidData(	( "#Error: Missing lwoCurveMarketData name" ), __FILE__, __LINE__ );
		}

		// Attempt to retrieve LWOCurveMarketData object from the LWO Cache
		auto lwoCurveMarketData = etrading::getCurveMarketData( lwoCurveMarketDataName );
  
		// Attempt to retrieve LWOCurveGenerator object from the LWO Cache
		auto lwoCurveGenerator = etrading::getCurveGenerator( lwoCurveGeneratorName );
       
		// Check for matching IdentityParams from the CurveGenerator and CurveMarketData
		const LabelValueBlock curvePropertiesLVB = lwoCurveGenerator->toLabelValueBlock( "CURVEPROPERTIES" );
		const std::string configCurrency         = curvePropertiesLVB.getCompulsoryValue( "Currency" );
		const std::string configCurveType        = curvePropertiesLVB.getCompulsoryValue( "CurveType" );
		
		const LabelValueBlock marketDataPropertiesLVB = lwoCurveMarketData->toLabelValueBlock( "MARKETDATAPROPERTIES" );
		const std::string marketDataCurrency          = marketDataPropertiesLVB.getCompulsoryValue( "Currency" );
		const std::string marketDataCurveType         = marketDataPropertiesLVB.getCompulsoryValue( "CurveType" );
		
        etrading::CurveTypeEnum curveTypeEnum = etrading::toCurveTypeEnum( configCurveType );

        if ( configCurrency != marketDataCurrency )
		{
			throw LACoreInvalidData( ( boost::format("#Error: CurveGenerator currency \"%s\" does not match MarketData Currency \"%s\"" )
                                   % configCurrency % marketDataCurrency ).str().c_str(), __FILE__, __LINE__ );
		}
		if ( configCurveType != marketDataCurveType )
		{
			throw LACoreInvalidData( ( boost::format("#Error: CurveGenerator CurveType \"%s\" does not match MarketData CurveType \"%s\"" )
                                   % configCurveType % marketDataCurveType ).str().c_str(), __FILE__, __LINE__ );
		}
		if (curveTypeEnum != etrading::FX_CURVETYPE)
		{
			throw LACoreInvalidData( ( boost::format("#Error: CurveGenerator CurveType \"%s\" is not FX" )
                                   % configCurveType ).str().c_str(), __FILE__, __LINE__ );
		}

		// Create the FxCurve object
        etrading::FXCurve fxCurve(objectName, lwoCurveGenerator, lwoCurveMarketData);

        // ..  and store in the cache
        etrading::copyToCache<etrading::FXCurve>( fxCurve );

		// Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( objectName);

		VALID_EXCEPTION_END

    }


    /* @brief validation interface for meFXForwardsFromDiscountCurves
	 * @param [in] objectName           The objectName of the FxCurve
     * @param [in] settleDatesOrTenors  Settlement dates or tenors
     * @param [in] baseCurveCollection  Base currency curve collection
	 * @param [in] termCurveCollection  Term currency curve collection
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
 	 */
    AnyTypeMatrix tryMeLWOFXForwardsFromDiscountCurves(const std::string& objectName,
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

	/* @brief validation interface for meFXForwards
	 * @param [in] objectName           The objectName of the FxCurve
     * @param [in] settleDatesOrTenors  Settlement dates or tenors
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
 	 */
    AnyTypeMatrix tryMeLWOFXForwards(const std::string& objectName,
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


    /* @brief validation interface for meLWOFXSwapFromDiscountCurves
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
    AnyTypeMatrix tryMeLWOFXSwapFromDiscountCurves(const std::string& objectName,
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

	/* @brief validation interface for meLWOFXSwap
	 * @param [in] objectName						The objectName of the FxCurve
     * @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
     * @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
     * @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	 * @param [in] showColumnHeaders				True to show column headers, default to true
	 * @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]									Fx Swap outright rates and points
 	 */
    AnyTypeMatrix tryMeLWOFXSwap(const std::string& objectName,
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





	/* @brief validation interface for meLWOFXForwardsFromXccyCurveObject
	 * @param [in] xccyCurveObjectName  xccyCurveObjectName
	 * @param [in] settleDatesOrTenors  Settlement dates or tenors
	 * @param [in] swapRateSpread       XccyCurve SwapRate Spread
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
	 */
	AnyTypeMatrix tryMeLWOFXForwardsFromXccyCurveObject(const std::string& xccyCurveObjectName,
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

	/* @brief validation interface for meLWOFXForwardsFromXccyCurve
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
	AnyTypeMatrix tryMeLWOFXForwardsFromXccyCurve(const std::string& baseCurveCollection,
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

	/* @brief validation interface for meLWOFXSwapFromXccyCurve
	* @param [in] xccyCurveObjectName				xccyCurveObjectName
	* @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
	* @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
	* @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	* @param [in] showColumnHeaders				True to show column headers, default to true
	* @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	* @param [out]									Fx Swap outright rates and points
	*/
	AnyTypeMatrix tryMeLWOFXSwapFromXccyCurveObject(const std::string& xccyCurveObjectName,
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

	
	/* @brief validation interface for meLWOFXSwapFromXccyCurve
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
	AnyTypeMatrix tryMeLWOFXSwapFromXccyCurve(const std::string& baseCurveCollection,
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

	/*	@brief		validation interface for the meLWOFXSpotToAsofDate method
	*	@param [in]	fxSpot					FX rate on FX Spot Date
	*	@param [in]	fxSpotDate				FX Spot Date
	*	@param [in]	baseCurveCollection		baseCurveCollection
	*	@param [in]	baseDiscountCurveIndex	baseDiscountCurveIndex
	*	@param [in]	termCurveCollection		termCurveCollection
	*	@param [in]	termDiscountCurveIndex	termDiscountCurveIndex
	*	@return		FxRate on curve as of date
	*/
	double tryMeLWOFXSpotToAsOfDate(const double fxSpot, const LADate& fxSpotDate, const std::string& baseCurveCollection, const std::string& baseDiscountCurveIndex, const std::string& termCurveCollection, const std::string& termDiscountCurveIndex)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(fxSpot, fxSpotDate, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex);

		double ret = etrading::fxRateFromSpotToAsOfDate(fxSpot, fxSpotDate, baseCurveCollection, baseDiscountCurveIndex, termCurveCollection, termDiscountCurveIndex);

		// Record Outputs AND Return the Result for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT(ret);

		VALID_EXCEPTION_END
	}

	/*	@brief		validation interface for the meLWOFXAsofDateToSpot method
	*	@param [in]	fxAsOfDateRate			FX rate on As of Date
	*	@param [in]	fxSpotDate				FX Spot Date
	*	@param [in]	baseCurveCollection		baseCurveCollection
	*	@param [in]	baseDiscountCurveIndex	baseDiscountCurveIndex
	*	@param [in]	termCurveCollection		termCurveCollection
	*	@param [in]	termDiscountCurveIndex	termDiscountCurveIndex
	*	@return		FxRate on curve as of date
	*/
	double tryMeLWOFXAsOfDateToSpot(const double fxAsOfDateRate, const LADate& fxSpotDate, const std::string& baseCurveCollection, const std::string& baseDiscountCurveIndex, const std::string& termCurveCollection, const std::string& termDiscountCurveIndex)
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