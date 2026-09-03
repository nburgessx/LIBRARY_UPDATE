#include "InitializeAQETrading.h"
#include "mirGetForwardRate.h"
#include "TypeUtilities.h"
#include "AQLString.h"
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

		AQLString cId(curveId.c_str());
		AQLString freq(frequency.c_str());
		AQLString dayC(dayCount.c_str());
		AQLString sRule(slidingRule.c_str());
		AQLString cal(calendar.c_str());
		AQLString interp(interpolation.c_str());
		AQLString cName(curveName.c_str());

		results = validation::tryMirGetForwardRate2(etrading::InitializeAQETrading::instance().dataInstance(), fromDateVec, toDateVec, cId, freq, dayC, sRule, cal, interp, cName, isFwdInterp, useFwdData);

	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return results;
}
