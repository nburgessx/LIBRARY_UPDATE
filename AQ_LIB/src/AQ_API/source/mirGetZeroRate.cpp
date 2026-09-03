#include "InitializeAQETrading.h"
#include "mirGetZeroRate.h"
#include "TypeUtilities.h"
#include "LAString.h"
#include "tryMirGetZeroRate.h"



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
						bool isFwdInterp)
{
	double ret(0.);
	try 
	{
		// Input marshalling
		DoubleVector terms;
		terms.push_back(Term);

		LAString cId(curveId.c_str());
		LAString freq(frequency.c_str());
		LAString dayC(dayCount.c_str());
		LAString interp(interpolation.c_str());
		LAString cName(curveName.c_str());

		DoubleVector results = validation_api::tryMirGetZeroRate1(etrading::InitializeAQETrading::instance().dataInstance(), terms, cId, freq, dayC, interp, cName, isFwdInterp);
		ret = results[0];

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


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
									bool isFwdInterp)
{
	std::vector<double> ret;
	try 
	{
		// Input marshalling
		LAString cId(curveId.c_str());
		LAString freq(frequency.c_str());
		LAString dayC(dayCount.c_str());
		LAString interp(interpolation.c_str());
		LAString cName(curveName.c_str());

		ret = validation_api::tryMirGetZeroRate1(etrading::InitializeAQETrading::instance().dataInstance(), Terms, cId, freq, dayC, interp, cName, isFwdInterp);

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}

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
						bool isFwdInterp)
{
	double ret(0.0);
	try 
	{
		LAStringVector terms;
		terms.push_back(Term.c_str());

		// Input marshalling
		LAString cId(curveId.c_str());
		LAString freq(frequency.c_str());
		LAString dayC(dayCount.c_str());
		LAString interp(interpolation.c_str());
		LAString slideRule(slidingRule.c_str());
		LAString cal(calendar.c_str());
		LAString cName(curveName.c_str());

		std::vector<double> results = validation_api::tryMirGetZeroRate2(etrading::InitializeAQETrading::instance().dataInstance(), terms, cId, freq, dayC, slideRule, cal, interp, cName, isFwdInterp);
		ret = results[0];

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}

/* @brief			swig interface for mirGetZeroRate2
*  @param [in]		Terms			A  list of given terms in the form of tenor string
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
									bool isFwdInterp)
{
	std::vector<double> ret;
	try 
	{
		LAStringVector terms;
		swig::buildStringVector(terms, Terms);

		// Input marshalling
		LAString cId(curveId.c_str());
		LAString freq(frequency.c_str());
		LAString dayC(dayCount.c_str());
		LAString interp(interpolation.c_str());
		LAString slideRule(slidingRule.c_str());
		LAString cal(calendar.c_str());
		LAString cName(curveName.c_str());

		ret = validation_api::tryMirGetZeroRate2(etrading::InitializeAQETrading::instance().dataInstance(), terms, cId, freq, dayC, slideRule, cal, interp, cName, isFwdInterp);

	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}

