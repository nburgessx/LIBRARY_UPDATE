#pragma once

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for aqCurveForwardRatesFromYearFraction
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		yearFraction	YearFraction
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set. Default to STD
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		fwdInter		True or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust	The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on fromDates and yearFraction
    */
    DoubleVector tryAqCurveForwardRatesFromYearFraction( const DateVector& fromDates,
														double yearFraction,
														const AQLString& dayCount,
														const AQLString& curveCollection,
														const AQLString& curveIndex, 
														const std::string& fwdInter="",
														const std::string& businessDayAdjust="");

    /* @brief			validation interface for aqCurveForwardRates
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set. Default to STD
    *  @param [in]		fwdInter		fwdInter flag, true or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust	The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqCurveForwardRatesFromForwardDates( const DateVector& fromDates,
														const DateVector& toDates,
														const AQLString& curveCollection,
														const AQLString& curveIndex, 
														const std::string& fwdInter="",
														const std::string& businessDayAdjust = "");

	/* @brief			validation interface for aqCurveForwardRates
    *  @param [in]		fromDates					A vector of from dates
    *  @param [in]		toDates						A vector of to dates
    *  @param [in]		curveCollectionOrHandle		ID of the yield curve
    *  @param [in]		curveIndex					Index of the curve set. Default to STD
    *  @param [in]		fwdInter					fwdInter flag, true or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust			The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqCurveForwardRatesFromForwardDatesFromObject( const DateVector& fromDates,
																   const DateVector& toDates,
																   const AQLString& curveCollectionOrHandle,
																   const AQLString& curveIndex, 
																   const std::string& fwdInter="",
																   const std::string& businessDayAdjust = "" );

    /* @brief			Validation interface for aqCurveForwardRates, which uses the curve frequency
    *  @param [in]		fixingDates			A vector of fixing dates
    *  @param [in]		curveCollection		ID of the yield curve
    *  @param [in]		curveIndex			Index of the curve set. Default to STD
    *  @param [in]		fwdInter			True or false, if not specified use the curve one
	*  @param [in]		businessDayAdjust	The business day adjustment, Default to MOD_FOLLOWING
	*  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqCurveForwardRates( const DateVector& fixingDates,
                                         const AQLString& curveCollectionOrHandle,
                                         const AQLString& curveIndex, 
										 const std::string& fwdInter="",
										 const std::string& businessDayAdjust="");

	/* @brief			Validation interface for aqCurveObjectForwardRatesTable, which generates a date schedule and
	*					uses the date schedule to calculate a matrix of forward rates for a set of curveIndices.
	*  @param [out]		fixingDates			The fixing dates on which the forwardRates are calculated. Generated from schedule parameters
	*  @param [out]		forwardRates		A matrix containing the calculated forward rates for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating forward rates
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY		
    *  @param [in]		fwdInterps			A list of fwdInter Flags for the curveIndices. Default to empty
	*/
	void tryAqCurveObjectForwardRatesTable( DateVector& fixingDates,
										 DoubleMatrix& forwardRates,
										 const AQLString& curveCollection,
										 const AQLStringVector& curveIndices,
										 const AQLString& startDate,
										 const AQLString& maturity,
										 const AQLString& businessDayAdjust,
										 const AQLString& calendar,
										 const AQLString& rollConvention,
										 const AQLString& frequency, 
										 const AQLStringVector& fwdInterps=AQLStringVector());
 }
