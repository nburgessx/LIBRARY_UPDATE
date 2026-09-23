// aqCDSObject.cpp

#include "aqCDSObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwapObjectPricing.h"  // validation::tryAqCDSObject...
#include "APISetUp.h"                // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"     // etrading::stringToDate

double aqCDSObjectPVFromHazardRate( const std::string& swapName,
                                     const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                     const double hazardRate,
                                     const double recoveryRate,
                                     const std::string& legName,
                                     const bool includeAccruedInterest )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqCDSObjectPVFromHazardRate(
        swapName, valuationSettingsLVB_, hazardRate, recoveryRate, legName.c_str(), includeAccruedInterest );
    return result;

    AQ_API_END
}

double aqCDSObjectPVByIntegration( const std::string& swapName,
                                    const std::string& creditModelName,
                                    const std::string& legName,
                                    const size_t numberOfIntegrationPoints,
                                    const bool evaluateInParallel,
                                    const bool payDefaultCashflowsOnNextCouponDate )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqCDSObjectPVByIntegration(
        swapName, creditModelName, legName.c_str(), numberOfIntegrationPoints,
        evaluateInParallel, payDefaultCashflowsOnNextCouponDate );
    return result;

    AQ_API_END
}

std::vector<double> aqCDSObjectPVByMonteCarlo( const std::string& swapName,
                                                const std::string& creditModelName,
                                                const std::string& legName,
                                                const std::vector<std::vector<std::string> >& mcParametersLVB,
                                                const bool payDefaultCashflowsOnNextCouponDate )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock mcParametersLVB_ = swig::buildSingleLabelValueBlock( mcParametersLVB );

    // Call Function
    double standardError = 0.0;
    double pv = validation::tryAqCDSObjectPVByMonteCarlo(
        swapName, creditModelName, legName.c_str(), mcParametersLVB_,
        payDefaultCashflowsOnNextCouponDate, standardError );

    // Marshall Output(s) - [PV, Monte-Carlo standard error], matching the AQ_XLL shape
    std::vector<double> result;
    result.push_back( pv );
    result.push_back( standardError );
    return result;

    AQ_API_END
}

double aqCDSObjectRiskyAnnuityFromHazardRate( const std::string& swapName,
                                               const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                               const double hazardRate,
                                               const double recoveryRate,
                                               const std::string& legName,
                                               const bool includeAccruedInterest )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqCDSObjectRiskyAnnuityFromHazardRate(
        swapName, valuationSettingsLVB_, hazardRate, recoveryRate, legName.c_str(), includeAccruedInterest );
    return result;

    AQ_API_END
}

double aqCDSObjectAccruedYearFraction( const std::string& swapName,
                                        const std::string& creditModelName,
                                        const std::string& toDate,
                                        const std::string& legName )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate toDate_( etrading::stringToDate( toDate ) );

    // Call Function and Return Result
    double result = validation::tryAqCDSObjectAccruedYearFraction( swapName, creditModelName, toDate_, legName.c_str() );
    return result;

    AQ_API_END
}

double aqCDSObjectParSpreadFromHazardRate( const std::string& swapName,
                                            const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                            const double hazardRate,
                                            const double recoveryRate,
                                            const std::string& premiumLegName,
                                            const std::string& protectionLegName,
                                            const bool includeAccruedInterest )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqCDSObjectParSpreadFromHazardRate(
        swapName, valuationSettingsLVB_, hazardRate, recoveryRate,
        premiumLegName.c_str(), protectionLegName.c_str(), includeAccruedInterest );
    return result;

    AQ_API_END
}

double aqCDSObjectHazardRateFromParSpread( const std::string& swapName,
                                            const std::vector<std::vector<std::string> >& valuationSettingsLVB,
                                            const double parSpread,
                                            const double recoveryRate,
                                            const std::string& premiumLegName,
                                            const std::string& protectionLegName,
                                            const bool includeAccruedInterest )
{
    AQ_API_START

    // Marshall Inputs
    LabelValueBlock valuationSettingsLVB_ = swig::buildSingleLabelValueBlock( valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqCDSObjectHazardRateFromParSpread(
        swapName, valuationSettingsLVB_, parSpread, recoveryRate,
        premiumLegName.c_str(), protectionLegName.c_str(), includeAccruedInterest );
    return result;

    AQ_API_END
}
