/*
 * @brief			validation interface for meLWOCurveDiscountFactor method(s)
 * @Created:		19 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <boost/date_time.hpp>
#include "LACoreTemplateType.h"

namespace validation_api
{
    /* @brief			validation method for meCurveDiscountFactorsFromYearFractions
    *  @param [in]		lwoCurveName	Curve Name
    *  @param [in]		yearFractions	An array of yearFractions by year count
    *  @param [in]		dayCount	    The daycount fraction used to generate the yearFraction
    *  @return			An array of discount factor
    */
    DoubleVector tryMeLWOCurveDiscountFactorsFromYearFractions( const std::string& lwoCurveName,
            const DoubleVector& yearFractions,
            const LAString& dayCount );

    /* @brief			validation method for meCurveLWODiscountFactorsFromTenors
    *  @param [in]		lwoCurveName	Curve Name
    *  @param [in]		tenors			An array of tenor strings
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			An array of discount factor
    */
    DoubleVector tryMeLWOCurveDiscountFactorsFromTenors( const std::string& lwoCurveName,
            const std::vector< std::string >& tenors,
            const std::string& businessDayAdj  = "NO_CHANGE",
            const std::string& calendar = "" );

    /* @brief			validation method for meCurveDiscountFactors
    *  @param [in]		lwoCurveName	Curve Name
    *  @param [in]		paymentDates			A single or an array of payment Dates in YYYYMMDD formate
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactors( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& paymentDates );


    /* @brief			validation method for meCurveDiscountFactorsForwardStarting
    *  @param [in]		lwoCurveName	LWO Curve Name
    *  @param [in]		fromDates		A single array of dates in YYYYMMDD formate
    *  @param [in]		toDates			An array of dates in YYYYMMDD formate
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactorsForwardStarting( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates );

    /* @brief			validation method for meCurveDiscountFactorsForwardStartingFromYearFractions
    *  @param [in]		lwoCurveName	LWO Curve Name
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactorsForwardStartingFromYearFractions( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const DoubleVector& yearFractions,
            const LAString& dayCount );

    /* @brief			validation method for meCurveDiscountFactorsForwardStartingFromTenor
    *  @param [in]		lwoCurveName	    LWO Curve Name
    *  @param [in]		fromDates		    An array of from-dates in YYYYMMDD formate
    *  @param [in]		tenor			    A tenor string
    *  @param [in]		businessDayAdj	    Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		    Calendar
    *  @return			A array of discount factors
    */
    DoubleVector tryMeLWOCurveDiscountFactorsForwardStartingFromTenors( const std::string& lwoCurveName,
                                                                        const std::vector<boost::gregorian::date>& fromDates,
                                                                        const std::vector<std::string>& tenors,
                                                                        const std::string& businessDayAdj,
                                                                        const std::string& calendar );
    
    /* @brief			Validation interface for meLWOCurveDiscountFactorsTable, which generates a date schedule and
	*					uses the date schedule to calculate a matrix of discount factors for a set of curveIndices.
	*  @param [out]		paymentDates		The payment dates on which the discountFactors are calculated. Generated from schedule parameters
	*  @param [out]		discountFactors		A matrix containing the calculated discount factors for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating discount factors
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY		
	*/
	void tryMeLWOCurveDiscountFactorsTable( DateVector& paymentDates,
											DoubleMatrix& discountFactors,
											const LAString& curveCollection,
											const LAStringVector& curveIndices,
											const LAString& startDate,
											const LAString& maturity,
											const LAString& businessDayAdjust,
											const LAString& calendar,
											const LAString& rollConvention,
											const LAString& frequency );


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
	DoubleVector tryMeLWOCurveDiscountFactorsWithSpread( const DateVector& paymentDates,
														 const std::string& curveCollection,
														 const std::string& curveIndex,
														 const double spread,
														 const std::string& fixingTableName );

}

