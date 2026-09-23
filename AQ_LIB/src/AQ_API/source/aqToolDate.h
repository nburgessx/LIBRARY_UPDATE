// aqToolDate.h

/*
 * @brief			Swig interface for aqToolTermsToDates and aqToolDatesToTerms
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for aqToolTermsToDates. Convert a list of year-fraction terms, measured
*                   from AsOfDate, into dates.
*  @param [in]		asOfDate	The anchor date
*  @param [in]		terms		A vector of year-fraction terms
*  @return			The corresponding dates
*/
std::vector<std::string> aqToolTermsToDates( const std::string& asOfDate, const std::vector<double>& terms );

/* @brief			swig interface for aqToolDatesToTerms. Convert a list of dates into year-fraction terms
*                   measured from AsOfDate.
*  @param [in]		asOfDate		The anchor date
*  @param [in]		paymentDates	A vector of dates
*  @return			The corresponding year-fraction terms
*/
std::vector<double> aqToolDatesToTerms( const std::string& asOfDate, const std::vector<std::string>& paymentDates );
