/* 
 * @brief			Swig interface to Java for mirGetZeroRate methods
 * @Created:		31 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for mirGetZeroRate1
*  @param [in]		Term			A year fraction term
*  @param [in]		curveID			ID of the yield curve
*  @param [in]		frequency		Frequency of the yield curve
*  @param [in]		dayCount		Day count convention of the yield curve
*  @param [in]		interpolation	Interpolation method of the yield curve
*  @param [in]		curveName		Type of the yield curve, default to STD
*  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
*  @return			Zero rate over a given term from the curve's asof date 
*/
double mirGetZeroRate1(double Term, 
						const std::string& curveId, 
						const std::string& frequency, 
						const std::string& dayCount, 
						const std::string& interpolation,
						const std::string& curveName, 
						bool isFwdInterp) throw(std::exception);


/* @brief			swig interface for mirGetZeroRate1
*  @param [in]		Terms			A  list of given terms in the form of year fraction
*  @param [in]		curveID			ID of the yield curve
*  @param [in]		frequency		Frequency of the yield curve
*  @param [in]		dayCount		Day count convention of the yield curve
*  @param [in]		interpolation	Interpolation method of the yield curve
*  @param [in]		curveName		Type of the yield curve, default to STD
*  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
*  @return			A list of zero rates over a list of given terms from the curve's asof date 
*/
std::vector<double> mirGetZeroRate1(const std::vector<double>& Terms, 
									const std::string& curveId, 
									const std::string& frequency, 
									const std::string& dayCount, 
									const std::string& interpolation,
									const std::string& curveName, 
									bool isFwdInterp) throw(std::exception);


/* @brief			swig interface for mirGetZeroRate2
*  @param [in]		Term			A single tenor string
*  @param [in]		curveID			ID of the yield curve
*  @param [in]		frequency		Frequency of the yield curve
*  @param [in]		dayCount		Day count convention of the yield curve
*  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar		Calendar used to calculate zero date
*  @param [in]		interpolation	Interpolation method of the yield curve
*  @param [in]		curveName		Type of the yield curve, default to STD
*  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
*  @return			Zero rate over a given term from the curve's asof date 
*/
double mirGetZeroRate2(const std::string& Term, 
						const std::string& curveId, 
						const std::string& frequency, 
						const std::string& dayCount, 
						const std::string& slidingRule, 
						const std::string& calendar, 
						const std::string& interpolation,
						const std::string& curveName, 
						bool isFwdInterp) throw(std::exception);


/* @brief			swig interface for mirGetZeroRate2
*  @param [in]		Terms			A  list of given terms
*  @param [in]		curveID			ID of the yield curve
*  @param [in]		frequency		Frequency of the yield curve
*  @param [in]		dayCount		Day count convention of the yield curve
*  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar		Calendar used to calculate zero dates
*  @param [in]		interpolation	Interpolation method of the yield curve
*  @param [in]		curveName		Type of the yield curve, default to STD
*  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
*  @return			A list of zero rates over a list of given terms from the curve's asof date 
*/
std::vector<double> mirGetZeroRate2(const std::vector<std::string>& Terms, 
									const std::string& curveId, 
									const std::string& frequency, 
									const std::string& dayCount, 
									const std::string& slidingRule, 
									const std::string& calendar, 
									const std::string& interpolation,
									const std::string& curveName, 
									bool isFwdInterp) throw(std::exception);


