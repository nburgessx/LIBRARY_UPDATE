/* 
 * @brief			Swig interface to Java for function mirGetDF(x)
 * @Created:		24 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "InitializeMLibETrading.h"
#include "TypeUtilities.h"
#include "mirGetDF.h"
#include "LADate.h"
#include "LAString.h"
#include "tryMirGetDF.h"



/* @brief			swig interface for mirGetDF1
*  @param [in]		Term			The term by year count
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			A discount factor to the curve's asof date
*/
double mirGetDF1(double Term, 
				const std::string& CurveID, 
				const std::string& DayCount, 
				const std::string& Interpolation, 
				bool IsBasisFlag,
				const std::string& CurveName)
{
	double ret = 0.0;
	try 
	{
		// Input marshalling
		std::vector<double> terms;
		terms.push_back(Term);

		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		std::vector<double> DFs = validation_api::tryMirGetDF1(etrading::InitializeMLibETrading::instance().dataInstance(), terms, curveID, dayCount, interpolation, IsBasisFlag, curveName);
		ret = DFs[0];
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;
}

/* @brief			swig interface for mirGetDF1
*  @param [in]		Terms			An array of terms by year count
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			An array of discount factors to the curve's asof date
*/
std::vector<double> mirGetDF1(const std::vector<double>& Terms, 
								const std::string& CurveID, 
								const std::string& DayCount, 
								const std::string& Interpolation, 
								bool IsBasisFlag,
								const std::string& CurveName)
{
	std::vector<double> ret;
	try 
	{
		// Input marshalling
		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		ret = validation_api::tryMirGetDF1(etrading::InitializeMLibETrading::instance().dataInstance(), Terms, curveID, dayCount, interpolation, IsBasisFlag, curveName);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}

/* @brief			swig interface for mirGetDF2
*  @param [in]		Term			The term in the form of tenor string
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			A discount factor to the curve's asof date
*/
double mirGetDF2(const std::string& Term, 
				const std::string& CurveID, 
				const std::string& DayCount, 
				const std::string& SlidingRule, 
				const std::string& Calendar, 
				const std::string& Interpolation, 
				bool IsBasisFlag,
				const std::string& CurveName)
{
	double ret = 0.0;
	try 
	{
		// Input marshalling
		LAStringVector terms;
		terms.push_back(Term.c_str());

		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		std::vector<double> DFs = validation_api::tryMirGetDF2(etrading::InitializeMLibETrading::instance().dataInstance(), terms, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
		ret = DFs[0];
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}

/* @brief			swig interface for mirGetDF2
*  @param [in]		Terms			An array of tenor strings
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			An array of discount factors to the curve's asof date
*/
std::vector<double> mirGetDF2(const std::vector<std::string>& Terms, 
							const std::string& CurveID, 
							const std::string& DayCount, 
							const std::string& SlidingRule, 
							const std::string& Calendar, 
							const std::string& Interpolation, 
							bool IsBasisFlag,
							const std::string& CurveName)
{
	std::vector<double> ret;
	try 
	{
		// Input marshalling
		LAStringVector terms;
		swig::buildStringVector(terms, Terms);

		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		ret = validation_api::tryMirGetDF2(etrading::InitializeMLibETrading::instance().dataInstance(), terms, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}

/* @brief			swig interface for mirGetDF3
*  @param [in]		FromDate		A single from-date in YYYYMMDD formate 
*  @param [in]		ToDate			A single to-date in YYYYMMDD formate
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			A discount factor
*/
double mirGetDF3(const std::string& FromDate, 
				const std::string& ToDate, 
				const std::string& CurveID, 
				const std::string& DayCount, 
				const std::string& SlidingRule, 
				const std::string& Calendar, 
				const std::string& Interpolation, 
				bool IsBasisFlag,
				const std::string& CurveName)
{
	double ret = 0.0;
	try 
	{
		// Input marshalling
		DateVector fromDates;
		fromDates.push_back(LADate(FromDate.c_str(), "YYYYMMDD"));

		DateVector toDates;
		toDates.push_back(LADate(ToDate.c_str(), "YYYYMMDD"));

		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		std::vector<double> DFs = validation_api::tryMirGetDF3(etrading::InitializeMLibETrading::instance().dataInstance(), fromDates, toDates, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
		ret = DFs[0];
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


/* @brief			swig interface for mirGetDF3
*  @param [in]		FromDate		A single from-date in YYYYMMDD formate
*  @param [in]		ToDate			An array of single to-date in YYYYMMDD formate
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			A array of discount factors
*/
std::vector<double> mirGetDF3(const std::string& FromDate, 
				const std::vector<std::string>& ToDates, 
				const std::string& CurveID, 
				const std::string& DayCount, 
				const std::string& SlidingRule, 
				const std::string& Calendar, 
				const std::string& Interpolation, 
				bool IsBasisFlag,
				const std::string& CurveName)
{
	std::vector<double> ret;
	try 
	{
		// Input marshalling
		DateVector fromDates;
		fromDates.push_back(LADate(FromDate.c_str(), "YYYYMMDD"));

		DateVector toDates;
		swig::buildDateVector(toDates, ToDates);

		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		ret = validation_api::tryMirGetDF3(etrading::InitializeMLibETrading::instance().dataInstance(), fromDates, toDates, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


/* @brief			swig interface for mirGetDF4
*  @param [in]		FromDate		A from-dates in YYYYMMDD formate 
*  @param [in]		Term			A year fraction term
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			A discount factor
*/
double mirGetDF4(const std::string& FromDate, 
							double Term, 
							const std::string& CurveID, 
							const std::string& DayCount, 
							const std::string& SlidingRule, 
							const std::string& Calendar, 
							const std::string& Interpolation, 
							bool IsBasisFlag,
							const std::string& CurveName)
{
	double ret = 0.0;
	try 
	{
		// Input marshalling
		DateVector fromDates;
		fromDates.push_back(LADate(FromDate.c_str(), "YYYYMMDD"));

		DoubleVector terms;
		terms.push_back(Term);

		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		std::vector<double> DFs = validation_api::tryMirGetDF4(etrading::InitializeMLibETrading::instance().dataInstance(), fromDates, terms, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
		ret = DFs[0];
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


/* @brief			swig interface for mirGetDF4
*  @param [in]		FromDates		An array of from-dates in YYYYMMDD formate 
*  @param [in]		Terms			An array of year fraction terms
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			An array of discount factors
*/
std::vector<double> mirGetDF4(const std::vector<std::string>& FromDates, 
							const std::vector<double>& Terms, 
							const std::string& CurveID, 
							const std::string& DayCount, 
							const std::string& SlidingRule, 
							const std::string& Calendar, 
							const std::string& Interpolation, 
							bool IsBasisFlag,
							const std::string& CurveName)
{
	std::vector<double> ret;
	try 
	{
		// Input marshalling
		DateVector fromDates;
		swig::buildDateVector(fromDates, FromDates);

		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		ret = validation_api::tryMirGetDF4(etrading::InitializeMLibETrading::instance().dataInstance(), fromDates, Terms, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


/* @brief			swig interface for mirGetDF5
*  @param [in]		FromDate		A single from-date in YYYYMMDD formate 
*  @param [in]		TermTenor		A single tenor string
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			A discount factor
*/
double mirGetDF5(const std::string& FromDate, 
					const std::string& TermTenor, 
					const std::string& CurveID, 
					const std::string& DayCount, 
					const std::string& SlidingRule, 
					const std::string& Calendar, 
					const std::string& Interpolation, 
					bool IsBasisFlag,
					const std::string& CurveName)
{
	double ret = 0.0;
	try 
	{
		// Input marshalling
		DateVector fromDates;
		fromDates.push_back(LADate(FromDate.c_str(), "YYYYMMDD"));

		LAString term(TermTenor.c_str());
		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		std::vector<double> DFs = validation_api::tryMirGetDF5(etrading::InitializeMLibETrading::instance().dataInstance(), fromDates, term, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
		ret = DFs[0];
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


/* @brief			swig interface for mirGetDF5
*  @param [in]		FromDates		An array of from-dates in YYYYMMDD formate 
*  @param [in]		TermTenor		A single tenor string
*  @param [in]		CurveID			Curve set ID
*  @param [in]		DayCount		Day count convention. Default to ACT/365
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
*  @param [in]		Calendar		Calendar. Default to 'TKB:LNB'
*  @param [in]		Interpolation	Interpolation method. Default to 'SPLINE'
*  @param [in]		IsBasisFlag		Is using basis curve or not. Deprecated input
*  @param [in]		CurveName		Name of the curve where DFs are read off. Default to 'STD'
*  @return			An array of discount factors
*/
std::vector<double> mirGetDF5(const std::vector<std::string>& FromDates, 
					const std::string& TermTenor, 
					const std::string& CurveID, 
					const std::string& DayCount, 
					const std::string& SlidingRule, 
					const std::string& Calendar, 
					const std::string& Interpolation, 
					bool IsBasisFlag,
					const std::string& CurveName)
{
	std::vector<double> ret;
	try 
	{
		// Input marshalling
		DateVector fromDates;
		swig::buildDateVector(fromDates, FromDates);

		LAString term(TermTenor.c_str());
		LAString curveID(CurveID.c_str());
		LAString dayCount(DayCount.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString interpolation(Interpolation.c_str());
		LAString curveName(CurveName.c_str());

		ret = validation_api::tryMirGetDF5(etrading::InitializeMLibETrading::instance().dataInstance(), fromDates, term, curveID, dayCount, slidingRule, calendar, interpolation, IsBasisFlag, curveName);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}