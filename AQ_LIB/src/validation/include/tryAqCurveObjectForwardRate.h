#pragma once

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for aqCurveObjectForwardRatesFromYearFraction
    *  @param [in]		aqObjCurveName	AQObj Curve Name
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		yearFraction	YearFraction
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @return			The forward rates based on fromDates and yearFraction
    */
    DoubleVector tryAqCurveObjectForwardRatesFromYearFraction( const std::string& aqObjCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            double yearFraction,
            const std::string& dayCount );

    /* @brief			validation interface for tryAqCurveObjectForwardRatesFromForwardDates
    *  @param [in]		aqObjCurveName	AQObj Curve Name
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqCurveObjectForwardRatesFromForwardDates( const std::string& aqObjCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates );

    /* @brief			validation interface for aqCurveObjectForwardRates, which uses the curve frequency
    *  @param [in]		aqObjCurveName	AQObj Curve Name
    *  @param [in]		fixingDates	    A vector of fixing dates
    *  @return			The forward rates based on fixing dates
    */
    DoubleVector tryAqCurveObjectForwardRates( const std::string& aqObjCurveName,
                                            const std::vector<boost::gregorian::date>& fixingDates );

}
