/* 
 * @brief			Swig interface to Java for mirGetForwardRate methods
 * @Created:		24 March 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "InitializeMLibETrading.h"
#include "mirGetForwardRate.h"
#include "TypeUtilities.h"
#include "LAString.h"
#include "tryMirGetForwardRate.h"


/* @brief			swig interface for mirGetForwardRate2
*  @param [in]		fromDates	    A vector of from dates
*  @param [in]		toDates			A vector of to dates
*  @param [in]		curveID			ID of the yield curve
*  @param [in]		frequency		Frequency of the yield curve
*  @param [in]		dayCount		Day count convention of the yield curve
*  @param [in]		slidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		calendar		Calendar of the yield curve
*  @param [in]		interpolation	Interpolation method of the yield curve
*  @param [in]		curveName		Type of the yield curve, default to STD
*  @param [in]		isFwdInterp		Boolean that decides if direct interpolation on fwd rates is employed
*  @param [in]		useFwdData		Use False to imply forwards from Discount Factors and True to use Forward Data directly. It's default to False.
*  @return			The forward rates based on fromDates and toDates
*/
std::vector<double> mirGetForwardRate2( const std::vector<std::string>& fromDates, 
										const std::vector<std::string>& toDates, 
										const std::string& curveId, 
										const std::string& frequency, 
										const std::string& dayCount, 
										const std::string& slidingRule, 
										const std::string& calendar, 
										const std::string& interpolation,
										const std::string& curveName, 
										bool isFwdInterp, 
										bool useFwdData ) 
{
	std::vector<double> results;
	try 
	{
		// Input marshalling
		DateVector fromDateVec;
		swig::buildDateVector(fromDateVec, fromDates);
		DateVector toDateVec;
		swig::buildDateVector(toDateVec, toDates);

		LAString cId(curveId.c_str());
		LAString freq(frequency.c_str());
		LAString dayC(dayCount.c_str());
		LAString sRule(slidingRule.c_str());
		LAString cal(calendar.c_str());
		LAString interp(interpolation.c_str());
		LAString cName(curveName.c_str());

		results = validation_api::tryMirGetForwardRate2(etrading::InitializeMLibETrading::instance().dataInstance(), fromDateVec, toDateVec, cId, freq, dayC, sRule, cal, interp, cName, isFwdInterp, useFwdData);

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return results;
}
