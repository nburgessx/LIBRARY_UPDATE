// aqToolDate.cpp

/*
 * @brief			Swig interface for aqToolTermsToDates and aqToolDatesToTerms
 */

#include "aqToolDate.h"
#include "tryAqToolDate.h"

#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"

/* @brief			swig interface for aqToolTermsToDates. Convert a list of year-fraction terms, measured
*                   from AsOfDate, into dates.
*  @param [in]		asOfDate	The anchor date
*  @param [in]		terms		A vector of year-fraction terms
*  @return			The corresponding dates
*/
std::vector<std::string> aqToolTermsToDates( const std::string& asOfDate, const std::vector<double>& terms )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );
    DoubleVector terms_( terms );

    // Call Function
    DateVector dates = validation::tryAqToolTermsToDates( asOfDate_, terms_ );

    // Marshall Output(s)
    StandardStringVector result;
    swig::buildStringVectorFromDateVector( result, dates );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqToolDatesToTerms. Convert a list of dates into year-fraction terms
*                   measured from AsOfDate.
*  @param [in]		asOfDate		The anchor date
*  @param [in]		paymentDates	A vector of dates
*  @return			The corresponding year-fraction terms
*/
std::vector<double> aqToolDatesToTerms( const std::string& asOfDate, const std::vector<std::string>& paymentDates )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate asOfDate_( etrading::stringToDate( asOfDate ) );
    DateVector paymentDates_;
    swig::buildDateVector( paymentDates_, paymentDates );

    // Call Function and Return Result
    DoubleVector terms = validation::tryAqToolDatesToTerms( asOfDate_, paymentDates_ );
    return terms;

    AQ_API_END
}
