// aqMathRates.cpp

/*
 * @brief			Swig interface for the aqMathForwardRate / aqMathForwardRates and aqMathDiscountFactor / aqMathDiscountFactors low-level curve-fit primitives
 */

#include "aqMathRates.h"
#include "CoreEnumerations.h"        // etrading::toInterpolationEnum / toStateVariableEnum / toDayCountEnum / toBusinessDayAdjustmentEnum / toCompoundingFrequencyEnum
#include "TypeUtilities.h"
#include "ParameterValidation.h"    // etrading::stringToDate
#include "tryAqMathForwardRate.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

namespace
{
    // Local helper - build the AQLDate join-date argument, defaulting to an empty AQLDate() when blank
    AQLDate toJoinDateOr( const std::string& joinDate )
    {
        return joinDate.empty() ? AQLDate() : AQLDate( etrading::stringToDate( joinDate ) );
    }
}

double aqMathForwardRate( const std::string& fixingDate,
                           const std::string& asOfDate,
                           const std::string& interpolation,
                           const std::string& stateVariable,
                           const std::vector<std::string>& xValues,
                           const std::vector<double>& yValues,
                           const std::string& accrualDaycount,
                           const std::string& curveFrequencyTenor,
                           const std::string& fixingBusinessDayAdjustment,
                           const std::string& fixingCalendar,
                           const std::string& joinDate,
                           const unsigned int polynomialOrder,
                           const std::vector<std::vector<std::string> >& forwardAdjustmentTable,
                           const std::string& compoundFreq )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate fixingDate_( etrading::stringToDate( fixingDate ) );
    AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );

    DateVector xValues_;
    swig::buildDateVector( xValues_, xValues );

    // Call Function and Return Result
    double result = validation::tryAqMathForwardRate(
        fixingDate_, asOfDate_,
        etrading::toInterpolationEnum( interpolation ),
        etrading::toStateVariableEnum( stateVariable ),
        xValues_, yValues,
        etrading::toDayCountEnum( accrualDaycount ), curveFrequencyTenor,
        etrading::toBusinessDayAdjustmentEnum( fixingBusinessDayAdjustment ), fixingCalendar,
        toJoinDateOr( joinDate ),
        static_cast<size_t>( polynomialOrder ),
        forwardAdjustmentTable,
        etrading::toCompoundingFrequencyEnum( compoundFreq ) );
    return result;

    AQ_API_END
}

std::vector<double> aqMathForwardRates( const std::vector<std::string>& fixingDates,
                                         const std::string& asOfDate,
                                         const std::string& interpolation,
                                         const std::string& stateVariable,
                                         const std::vector<std::string>& xValues,
                                         const std::vector<double>& yValues,
                                         const std::string& accrualDaycount,
                                         const std::string& curveFrequencyTenor,
                                         const std::string& fixingBusinessDayAdjustment,
                                         const std::string& fixingCalendar,
                                         const std::string& joinDate,
                                         const unsigned int polynomialOrder,
                                         const std::vector<std::vector<std::string> >& forwardAdjustmentTable,
                                         const std::string& compoundFreq )
{
    AQ_API_START

    // Marshall Inputs
    DateVector fixingDates_;
    swig::buildDateVector( fixingDates_, fixingDates );

    AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );

    DateVector xValues_;
    swig::buildDateVector( xValues_, xValues );

    // Call Function and Return Result
    std::vector<double> result = validation::tryAqMathForwardRates(
        fixingDates_, asOfDate_,
        etrading::toInterpolationEnum( interpolation ),
        etrading::toStateVariableEnum( stateVariable ),
        xValues_, yValues,
        etrading::toDayCountEnum( accrualDaycount ), curveFrequencyTenor,
        etrading::toBusinessDayAdjustmentEnum( fixingBusinessDayAdjustment ), fixingCalendar,
        toJoinDateOr( joinDate ),
        static_cast<size_t>( polynomialOrder ),
        forwardAdjustmentTable,
        etrading::toCompoundingFrequencyEnum( compoundFreq ) );
    return result;

    AQ_API_END
}

double aqMathDiscountFactor( const std::string& paymentDate,
                              const std::string& asOfDate,
                              const std::string& interpolation,
                              const std::string& stateVariable,
                              const std::vector<std::string>& xValues,
                              const std::vector<double>& yValues,
                              const std::string& accrualDaycount,
                              const std::string& curveFrequencyTenor,
                              const std::string& fixingBusinessDayAdjustment,
                              const std::string& fixingCalendar,
                              const std::string& joinDate,
                              const unsigned int polynomialOrder,
                              const std::vector<std::vector<std::string> >& forwardAdjustmentTable,
                              const std::string& compoundFreq )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate paymentDate_( etrading::stringToDate( paymentDate ) );
    AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );

    DateVector xValues_;
    swig::buildDateVector( xValues_, xValues );

    // Call Function and Return Result
    double result = validation::tryAqMathDiscountFactor(
        paymentDate_, asOfDate_,
        etrading::toInterpolationEnum( interpolation ),
        etrading::toStateVariableEnum( stateVariable ),
        xValues_, yValues,
        etrading::toDayCountEnum( accrualDaycount ), curveFrequencyTenor,
        etrading::toBusinessDayAdjustmentEnum( fixingBusinessDayAdjustment ), fixingCalendar,
        toJoinDateOr( joinDate ),
        static_cast<size_t>( polynomialOrder ),
        forwardAdjustmentTable,
        etrading::toCompoundingFrequencyEnum( compoundFreq ) );
    return result;

    AQ_API_END
}

std::vector<double> aqMathDiscountFactors( const std::vector<std::string>& paymentDates,
                                            const std::string& asOfDate,
                                            const std::string& interpolation,
                                            const std::string& stateVariable,
                                            const std::vector<std::string>& xValues,
                                            const std::vector<double>& yValues,
                                            const std::string& accrualDaycount,
                                            const std::string& curveFrequencyTenor,
                                            const std::string& fixingBusinessDayAdjustment,
                                            const std::string& fixingCalendar,
                                            const std::string& joinDate,
                                            const unsigned int polynomialOrder,
                                            const std::vector<std::vector<std::string> >& forwardAdjustmentTable,
                                            const std::string& compoundFreq )
{
    AQ_API_START

    // Marshall Inputs
    DateVector paymentDates_;
    swig::buildDateVector( paymentDates_, paymentDates );

    AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );

    DateVector xValues_;
    swig::buildDateVector( xValues_, xValues );

    // Call Function and Return Result
    std::vector<double> result = validation::tryAqMathDiscountFactors(
        paymentDates_, asOfDate_,
        etrading::toInterpolationEnum( interpolation ),
        etrading::toStateVariableEnum( stateVariable ),
        xValues_, yValues,
        etrading::toDayCountEnum( accrualDaycount ), curveFrequencyTenor,
        etrading::toBusinessDayAdjustmentEnum( fixingBusinessDayAdjustment ), fixingCalendar,
        toJoinDateOr( joinDate ),
        static_cast<size_t>( polynomialOrder ),
        forwardAdjustmentTable,
        etrading::toCompoundingFrequencyEnum( compoundFreq ) );
    return result;

    AQ_API_END
}
