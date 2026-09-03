#include "InitializeAQETrading.h"
#include "mirGetAnnuity.h"
#include "TypeUtilities.h"
#include "tryMirGetAnnuity.h"


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
						const std::string& curveName)
{
	double ret = 0.0;
	try 
	{
		// marshall all inputs		
		LADate fromDt(fromDate.c_str(),"YYYYMMDD");
		LADate toDt (toDate.c_str(),"YYYYMMDD");
		LAString curId			(curveId.c_str());
		LAString freq			(frequency.c_str());
		LAString sRule			(slidingRule.c_str());
		LAString cal			(calendar.c_str());
		LAString firstOddDt		(firstOddDate.c_str());
		LAString lastOddDt		(lastOddDate.c_str());
		LAString paymentDy	    (paymentDay.c_str());
		LAString dayC			(dayCount.c_str());
		LAString interp			(interpolation.c_str());
		LAString curName		(curveName.c_str());
		
		ret = validation::tryMirGetAnnuity1(etrading::InitializeAQETrading::instance().dataInstance(),
												fromDt, 
												toDt, 
												curId, 
												freq, 
												sRule, 
												cal, 
												firstOddDt, 
												lastOddDt, 
												paymentDy,
												dayC, 
												interp, 
												curName);

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}

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
					const std::string& curveName)
{

	double ret = 0.0;
	try 
	{
		// Input marshalling
		DateVector dateVec;
		swig::buildDateVector(dateVec, dates);
		LAString curId(curveId.c_str());
		LAString dayC(dayCount.c_str());
		LAString interp(interpolation.c_str());
		LAString curName(curveName.c_str());

		ret = validation::tryMirGetAnnuity2(etrading::InitializeAQETrading::instance().dataInstance(),
								curId,
								dateVec,
								dayC,
								interp,
								curName);
		
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;
}