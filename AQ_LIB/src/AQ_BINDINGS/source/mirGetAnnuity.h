/* 
 * @brief			Swig interface to Java for mirGetAnnuity methods
 * @Created:		05 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include <vector>

/* @brief swig interface for mirGetAnnuity1 method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		curveId			ID of the curve set
*  @param [in]		frequency		Frequency
*  @param [in]		slidingRule		Sliding rule
*  @param [in]		calendar		Calendar
*  @param [in]		firstOddDate	End date of the front stub period
*  @param [in]		lastOddDate		Start date of the end stub period
*  @param [in]		paymentDay		PaymentDay
*  @param [in]		dayCount		Day count convention
*  @param [in]		interpolation	Interpolation method
*  @param [in]		curveName		Name of the curve set
*  @return			Annuity 
*/
double mirGetAnnuity1(const std::string& fromDate, 
						const std::string& toDate,
						const std::string& curveId, 
						const std::string& frequency,
						const std::string& slidingRule,
						const std::string& calendar,
						const std::string& firstOddDate, 
						const std::string& lastOddDate,
						const std::string& paymentDay,
						const std::string& dayCount, 
						const std::string& interpolation, 
						const std::string& curveName);

/* @brief swig interface for mirGetAnnuity2 method
*  @param [in]		curveId			ID of the curve set
*  @param [in]		dates			A vector of the payment dates
*  @param [in]		dayCount		Day count convention
*  @param [in]		interpolation	Interpolation method
*  @param [in]		curveName		Name of the curve set
*  @return			Annuity 
*/
double mirGetAnnuity2(const std::string& curveId, 
					const std::vector<std::string>& dates, 
					const std::string& dayCount, 
					const std::string& interpolation, 
					const std::string& curveName);