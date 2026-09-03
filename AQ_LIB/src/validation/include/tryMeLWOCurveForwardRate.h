#pragma once

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "LACoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for meLWOCurveForwardRatesFromYearFraction
    *  @param [in]		lwoCurveName	LWO Curve Name
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		yearFraction	YearFraction
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @return			The forward rates based on fromDates and yearFraction
    */
    DoubleVector tryMeLWOCurveForwardRatesFromYearFraction( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            double yearFraction,
            const std::string& dayCount );

    /* @brief			validation interface for tryMeLWOCurveForwardRatesFromForwardDates
    *  @param [in]		lwoCurveName	LWO Curve Name
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector tryMeLWOCurveForwardRatesFromForwardDates( const std::string& lwoCurveName,
            const std::vector<boost::gregorian::date>& fromDates,
            const std::vector<boost::gregorian::date>& toDates );

    /* @brief			validation interface for meLWOCurveForwardRates, which uses the curve frequency
    *  @param [in]		lwoCurveName	LWO Curve Name
    *  @param [in]		fixingDates	    A vector of fixing dates
    *  @return			The forward rates based on fixing dates
    */
    DoubleVector tryMeLWOCurveForwardRates( const std::string& lwoCurveName,
                                            const std::vector<boost::gregorian::date>& fixingDates );

}
