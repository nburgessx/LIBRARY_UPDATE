#pragma once

#include <boost/date_time.hpp>
#include "AQLCoreTemplateType.h"

// Forward Declaration, for definition see "AQLCurveCalibrationHelpers.h" was "AQLCurveCalibrationHelpers.h" 
struct DiscountFactorTable;

namespace validation
{
    
    /* @brief			return the day count used in the core library
    */
    AQLString getDayCount();

    /* @brief			function sets the legacy isBasisFlag parameter to false in all cases
    */
    bool isBasisFlag();


    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	curveIndex		curve index
    *  @param [inout]	interpolation	interpolation
    */
    void populateDiscountFactorConventions( const AQLString& curveCollection, AQLString& curveIndex, AQLString& interpolation );

    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	curveIndex		curve index
    *  @param [inout]	interpolation	interpolation
    *  @param [inout]	businessDayAdj	business day adjustment
    *  @param [inout]	calendar		calendar
    */
    void populateDiscountFactorConventions( const AQLString& curveCollection, AQLString& curveIndex, AQLString& interpolation, AQLString& businessDayAdj, AQLString& calendar );



    /* @brief			validation method for aqCurvesDiscountFactorsFromYearFractions
    *  @param [in]      yearFractions	An array of yearFractions by year count
    *  @param [in]		dayCount	    The daycount fraction used to generate the yearFraction
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			An array of discount factor
    */
    DoubleVector tryAqCurvesDiscountFactorsFromYearFractions( const DoubleVector& yearFractions,
                                                             const AQLString& dayCount,
                                                             const AQLString& curveCollection,
                                                             const AQLString& curveIndex );

    /* @brief			validation method for aqCurvesDiscountFactorsFromTenors
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			An array of discount factor
    */
    DoubleVector tryAqCurvesDiscountFactorsFromTenors( const AQLStringVector& tenors,
                                                      const AQLString& businessDayAdj,
                                                      const AQLString& calendar,
                                                      const AQLString& curveCollection,
                                                      const AQLString& curveIndex );

    /* @brief			validation method for aqCurvesDiscountFactors
    *  @param [in]		toDates			A single or an array of to-date in YYYYMMDD formate
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurvesDiscountFactors( const DateVector& toDates,
                                            const AQLString& curveCollectionOrHandle,
                                            const AQLString& curveIndex );

    /* @brief			validation method for aqCurvesDiscountFactorsForwardStarting
    *  @param [in]		fromDates		A single array of dates in YYYYMMDD formate
    *  @param [in]		toDates			An array of dates in YYYYMMDD formate
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurvesDiscountFactorsForwardStarting( const DateVector& fromDates,
                                                           const DateVector& toDates,
                                                           const AQLString& curveCollection,
                                                           const AQLString& curveIndex );

    /* @brief			validation method for aqCurvesDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]	    yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @param [in]		curveCollection		Curve set ID
    *  @param [in]		curveIndex			Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar			Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurvesDiscountFactorsForwardStartingFromYearFractions( const DateVector& fromDates,
                                                                            const DoubleVector& yearFractions,
                                                                            const AQLString& dayCount,
                                                                            const AQLString& curveCollection,
                                                                            const AQLString& curveIndex );

    /* @brief			validation method for aqCurvesDiscountFactorsForwardStartingFromTenor
    *  @param [in]		fromDates		An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			A tenor string
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryAqCurvesDiscountFactorsForwardStartingFromTenor( const DateVector& fromDates,
                                                                    const AQLString& tenor,
                                                                    const AQLString& curveCollection,
                                                                    const AQLString& curveIndex,
                                                                    const AQLString& businessDayAdj,
                                                                    const AQLString& calendar );

    /* @brief			Validation interface for tryAqCurvesForwardRatesOverride function, which overrides curve forward rates by setting equivalent discount factors
	*  @param [in]		curveCollection		        The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			        The curveindex within the curve
	*  @param [in]		fixingDates                 The fixing dates corresponding to our forward rates
	*  @param [in]		forwardRates		        The new discount factors
    *  @param [in]		setCorrespondingForwards	Set the corresponding discount factors on STD curves, will set DFs = 1.0 on STD curves if false
	*/
    AQLString tryAqCurvesForwardRatesOverride( const AQLString& curveCollection,
                                             const AQLString& curveIndex,
                                             const DateVector& fixingDates,
                                             const DoubleVector& forwardRates,
                                             const bool setCorrespondingDiscountFactors = true );

    /* @brief			Validation interface for tryAqCurvesDiscountFactorsOverride function, which overrides curve discount factors
	*  @param [in]		curveCollection		        The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			        The curveindex within the curve
	*  @param [in]		paymentDates                The payment dates corresponding to our discount factors
	*  @param [in]		discountFactors		        The new discount factors		
    *  @param [in]		setCorrespondingForwards	Set the corresponding forwards on STD curves, will set Forwards on STD curves to zero if false. *** Note *** When true this involves reverse calibration solving and can fail to solve / converge for extreme data sets.
	*/
    AQLString tryAqCurvesDiscountFactorsOverride( const AQLString& curveCollection,
                                                const AQLString& curveIndex,
                                                const DateVector& paymentDates,
                                                const DoubleVector& discountFactors,
                                                const bool setCorrespondingForwards = true );

    /* @brief			Validation interface for tryAqCurvesDiscountFactorsSetToOne function, which sets all discount factors to one
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*/
    AQLString tryAqCurvesDiscountFactorsSetToOne( const AQLString& curveCollection, const AQLString& curveIndex );

    /* @brief			Validation interface for tryAqCurvesDiscountFactorsDisplay function, which retrieves all discount factors
	*  @param [in]		curveCollection		    The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			    The curveindex within the curve
	*  @param [out]		DiscountFactorTable     A discount factor table structure that contains paymentDates_ and discountFactors_
    */
    DiscountFactorTable tryAqCurvesDiscountFactorsDisplay( const AQLString& curveCollection, const AQLString& curveIndex );

    /* @brief			Validation interface for tryAqCurvesTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		curveCollection		    The curveCollection to use to get the curve asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryAqCurvesTermsToDates( const AQLString& curveCollection, const DoubleVector terms );

    /* @brief			Validation interface for tryAqCurvesDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		curveCollection		    The curveCollection to use to get the curve asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryAqCurvesDatesToTerms( const AQLString& curveCollection, const DateVector paymentDates );

}

