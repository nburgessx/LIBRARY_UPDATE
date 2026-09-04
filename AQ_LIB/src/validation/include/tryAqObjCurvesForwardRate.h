#pragma once

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for aqObjCurvesForwardRatesFromYearFraction
    *  @param [in]		aqoCurveName	AQO Curve Name
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		yearFraction	YearFraction
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @return			The forward rates based on fromDates and yearFraction
    */
    DoubleVector tryAqObjCurvesForwardRatesFromYearFraction( const std::string& aqoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            double yearFraction,
            const std::string& dayCount );

    /* @brief			validation interface for tryAqObjCurvesForwardRatesFromForwardDates
    *  @param [in]		aqoCurveName	AQO Curve Name
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryAqObjCurvesForwardRatesFromForwardDates( const std::string& aqoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates );

    /* @brief			validation interface for aqObjCurvesForwardRates, which uses the curve frequency
    *  @param [in]		aqoCurveName	AQO Curve Name
    *  @param [in]		fixingDates	    A vector of fixing dates
    *  @return			The forward rates based on fixing dates
    */
    DoubleVector tryAqObjCurvesForwardRates( const std::string& aqoCurveName,
                                            const std::vector<boost::gregorian::date>& fixingDates );

}
