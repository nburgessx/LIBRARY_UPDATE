#pragma once

#include <string>
#include <vector>
#include "AQLCoreTemplateType.h"

namespace validation
{


    /* @brief validation interface for aqObjectsFXCurveCreate, creating a FxCurve using a CurveGenerator object and a CurveMarketData object
	 * @param [in] objectName                The objectName of the FxCurve
     * @param [in] lwoCurveGeneratorName     The name of the AQOCurveGenerator object to use
	 * @param [in] lwoCurveMarketDataName    The name of the AQOCurveMarketData object to use
	 * @param [out]                          The created FxCurve objectName
	 */
    std::string tryAqObjectsFXCurveCreate(	const std::string& objectName,
										const std::string& lwoCurveGeneratorName,
										const std::string& lwoCurveMarketDataName );


    /* @brief validation interface for aqFXForwardsFromDiscountCurves
	 * @param [in] objectName           The objectName of the FxCurve
     * @param [in] settleDatesOrTenors  Settlement dates or tenors
     * @param [in] baseCurveCollection  Base currency curve collection
	 * @param [in] termCurveCollection  Term currency curve collection
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
 	 */
    AnyTypeMatrix tryAqObjectsFXForwardsFromDiscountCurves(const std::string& objectName,
        											const std::vector<std::string>& settleDatesOrTenors, 
													const std::string& baseCurveCollection, 
													const std::string& termCurveCollection,
													const bool showColumnHeaders=false,
													const std::vector<std::string>& columnList=std::vector<std::string>());


    /* @brief validation interface for aqFXForwards
	 * @param [in] objectName           The objectName of the FxCurve
     * @param [in] settleDatesOrTenors  Settlement dates or tenors
	 * @param [in] showColumnHeaders	True to show column headers, default to true
	 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]                     Fx forwards outright rates and points
 	 */
    AnyTypeMatrix tryAqObjectsFXForwards(const std::string& objectName,
        							const std::vector<std::string>& settleDatesOrTenors, 
									const bool showColumnHeaders=false,
								    const std::vector<std::string>& columnList=std::vector<std::string>());

    /* @brief validation interface for aqObjectsFXSwapFromDiscountCurves
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
    AnyTypeMatrix tryAqObjectsFXSwapFromDiscountCurves(const std::string& objectName,
        										const std::vector<std::string>& nearLegSettlementDatesOrTenors, 
                                                const std::vector<std::string>& farLegSettlementDatesOrTenors, 
                                                const std::string& baseCurveCollection, 
                                                const std::string& termCurveCollection,
												const bool outputFarLeg,
												const bool showColumnHeaders=false,
												 const std::vector<std::string>& columnList=std::vector<std::string>());

	/* @brief validation interface for aqObjectsFXSwap
	 * @param [in] objectName						The objectName of the FxCurve
     * @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
     * @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
     * @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	 * @param [in] showColumnHeaders				True to show column headers, default to true
	 * @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	 * @param [out]									Fx Swap outright rates and points
 	 */
    AnyTypeMatrix tryAqObjectsFXSwap(const std::string& objectName,
        						const std::vector<std::string>& nearLegSettlementDatesOrTenors, 
								const std::vector<std::string>& farLegSettlementDatesOrTenors, 
								const bool outputFarLeg,
								const bool showColumnHeaders=false,
								const std::vector<std::string>& columnList=std::vector<std::string>());

	/* @brief validation interface for aqObjectsFXForwardsFromXccyCurveObject
		 * @param [in] xccyCurveObjectName  xccyCurveObjectName
		 * @param [in] settleDatesOrTenors  Settlement dates or tenors
		 * @param [in] swapRateSpread       XccyCurve SwapRate Spread
		 * @param [in] showColumnHeaders	True to show column headers, default to true
		 * @param [in] columnList			Column header names to show specified columns. Default to empty list showing all columns.
		 * @param [out]                     Fx forwards outright rates and points
		 */
	AnyTypeMatrix tryAqObjectsFXForwardsFromXccyCurveObject(const std::string& xccyCurveObjectName,
													const std::vector<std::string>& settleDatesOrTenors,
													const double xccyCurveSwapRateBumpSize,
													const bool showColumnHeaders,
													const std::vector<std::string>& columnList);

	/* @brief validation interface for aqObjectsFXForwardsFromXccyCurve
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
	AnyTypeMatrix tryAqObjectsFXForwardsFromXccyCurve(const std::string& baseCurveCollection,
												const std::string& baseCurveIndex,
												const std::string& termCurveCollection,
												const std::string& termCurveIndex,
												const double fxSpotRate,
												const double pipSize,
												const double xccyCurveSwapRateBumpSize,
												const std::vector<std::string>& settleDatesOrTenors,
												const bool showColumnHeaders,
												const std::vector<std::string>& columnList);

	/* @brief validation interface for aqObjectsFXSwapFromXccyCurve
	* @param [in] xccyCurveObjectName				xccyCurveObjectName
	* @param [in] nearLegSettlementDatesOrTenors	Near leg settlement dates or tenors
	* @param [in] farLegSettlementDatesOrTenors	Far leg settlement dates or tenors
	* @param [in] outputFarLeg						True to output the far leg's rates, False to output the near leg's rates
	* @param [in] showColumnHeaders				True to show column headers, default to true
	* @param [in] columnList						Column header names to show specified columns. Default to empty list showing all columns.
	* @param [out]									Fx Swap outright rates and points
	*/
	AnyTypeMatrix tryAqObjectsFXSwapFromXccyCurveObject(const std::string& xccyCurveObjectName,
													const std::vector<std::string>& nearLegSettlementDatesOrTenors,
													const std::vector<std::string>& farLegSettlementDatesOrTenors,
													const bool outputFarLeg,
													const bool showColumnHeaders,
													const std::vector<std::string>& columnList);

	/* @brief validation interface for aqObjectsFXSwapFromXccyCurve
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
	AnyTypeMatrix tryAqObjectsFXSwapFromXccyCurve(const std::string& baseCurveCollection,
											const std::string& baseCurveIndex,
											const std::string& termCurveCollection,
											const std::string& termCurveIndex,
											const double fxSpotRate,
											const double pipSize,
											const std::vector<std::string>& nearLegSettlementDatesOrTenors,
											const std::vector<std::string>& farLegSettlementDatesOrTenors,
											const bool outputFarLeg,
											const bool showColumnHeaders,
											const std::vector<std::string>& columnList);

	
	/*	@brief		validation interface for the aqObjectsFXSpotToAsofDate method
	*	@param [in]	fxSpot					FX rate on FX Spot Date
	*	@param [in]	fxSpotDate				FX Spot Date
	*	@param [in]	baseCurveCollection		baseCurveCollection
	*	@param [in]	baseDiscountCurveIndex	baseDiscountCurveIndex
	*	@param [in]	termCurveCollection		termCurveCollection
	*	@param [in]	termDiscountCurveIndex	termDiscountCurveIndex
	*	@return		FxRate on curve as of date
	*/
	double tryAqObjectsFXSpotToAsOfDate(const double fxSpot, const AQLDate& fxSpotDate, const std::string& baseCurveCollection, const std::string& baseDiscountCurveIndex, const std::string& termCurveCollection, const std::string& termDiscountCurveIndex);

	/*	@brief		validation interface for the aqObjectsFXAsofDateToSpot method
	*	@param [in]	fxAsOfDateRate			FX rate on As of Date
	*	@param [in]	fxSpotDate				FX Spot Date
	*	@param [in]	baseCurveCollection		baseCurveCollection
	*	@param [in]	baseDiscountCurveIndex	baseDiscountCurveIndex
	*	@param [in]	termCurveCollection		termCurveCollection
	*	@param [in]	termDiscountCurveIndex	termDiscountCurveIndex
	*	@return		FxRate on curve as of date
	*/
	double tryAqObjectsFXAsOfDateToSpot(const double fxAsOfDateRate, const AQLDate& fxSpotDate, const std::string& baseCurveCollection, const std::string& baseDiscountCurveIndex, const std::string& termCurveCollection, const std::string& termDiscountCurveIndex);




}


