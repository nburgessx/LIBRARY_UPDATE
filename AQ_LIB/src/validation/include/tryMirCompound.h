#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{

    /* @brief			validation interface for the tryMirCompound3 method
       /* @brief			validation interface for the mirCompound3 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		startDates		A vector of start dates
    *  @param [in]		endDates		A vector of end dates
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		frequency		Frequency
    *  @param [in]		spread			Spread in basis points.
    *  @param [in]		isStartRoll		Roll dates: True if roll dates are calculated from start date, false from end date
    *  @param [in]		rollConvention	Roll conventions
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		compoundType	Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @return			interest rates calculated based on the startDates, endDates, and compound type
    */
    DoubleVector tryMirCompound3( AQLDataInstance* dataInstance,
                                  const DateVector& startDates,
                                  const DateVector& endDates,
                                  const AQLString& curveId,
                                  const AQLString& forecastCurve,
                                  const AQLString& frequency,
                                  double spread,
                                  bool isStartRoll,
                                  const AQLString& rollConvention,
                                  const AQLString& calendar,
                                  const AQLString& slidingRule,
                                  const AQLString& dayCount,
                                  const AQLString& interpolation,
                                  const AQLString& compoundType,
                                  const AQLString& firstOddDate,
                                  const AQLString& lastOddDate );


    /* @brief			validation interface for the mirCompound3 method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		startDate		Start date
    *  @param [in]		endDate			End date
    *  @param [in]		curveId			ID of the curve set
    *  @param [in]		forecastCurve	Forecasting curve
    *  @param [in]		frequency		Frequency
    *  @param [in]		spread			Spread in basis points.
    *  @param [in]		isStartRoll		Roll dates: True if roll dates are calculated from start date, false from end date
    *  @param [in]		rollConvention	Roll conventions
    *  @param [in]		calendar		Calendar
    *  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount		Day count convention
    *  @param [in]		interpolation	Interpolation method
    *  @param [in]		compoundType	Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstOddDate	End date of the front stub period
    *  @param [in]		lastOddDate		Start date of the end stub period
    *  @return			compounding rate calculated based on the startDate, endDate, and compound type
    */
    double tryMirCompound3( AQLDataInstance* dataInstance,
                            const AQLDate& startDate,
                            const AQLDate& endDate,
                            const AQLString& curveId,
                            const AQLString& forecastCurve,
                            const AQLString& frequency,
                            double spread,
                            bool isStartRoll,
                            const AQLString& rollConvention,
                            const AQLString& calendar,
                            const AQLString& slidingRule,
                            const AQLString& dayCount,
                            const AQLString& interpolation,
                            const AQLString& compoundType,
                            const AQLString& firstOddDate,
                            const AQLString& lastOddDate );
}

