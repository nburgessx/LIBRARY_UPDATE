#include "mirGetIMMDate.h"
#include "TypeUtilities.h"
#include "LADate.h"
#include "LAString.h"
#include "tryMirGetIMMDate.h"


/* @brief			swig interface for mirGetIMMDate1. 
*  @param [in]		Year			The year of the IMM date
*  @param [in]		Month			The month of the IMM date
*  @param [in]		Calendar		Calendar
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The IMM date on the given month
*/
std::string mirGetIMMDate1(int Year, 
						int Month, 
						const std::string& Calendar, 
						const std::string& SlidingRule)
{
	std::string ret;
	try 
	{
		LAString calendar(Calendar.c_str());
		if( calendar == LAString("") ) 
		{
			calendar = LAString("TKB:LNB");
		}

		LAString slidingrule(SlidingRule.c_str());
		if( slidingrule == LAString("") ) 
		{
			slidingrule=LAString("FOLLOWING");
		}

		ret = validation::tryMirGetIMMDate1(Year, Month, calendar, slidingrule).getCString();
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


/* @brief			swig interface for mirGetIMMDate2 
*  @param [in]		Year			The year of the IMM date
*  @param [in]		Number			The number'th IMM date of the year
*  @param [in]		Calendar		Calendar
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date of the year. n is defined by input Number.
*/
std::string mirGetIMMDate2(int Year, 
						int Number, 
						const std::string& Calendar, 
						const std::string& SlidingRule)
{
	std::string ret;
	try 
	{
		LAString calendar(Calendar.c_str());
		if( calendar == LAString("") ) 
		{
			calendar = LAString("TKB:LNB");
		}

		LAString slidingrule(SlidingRule.c_str());
		if( slidingrule == LAString("") ) 
		{
			slidingrule=LAString("FOLLOWING");
		}

		ret = validation::tryMirGetIMMDate2(Year, Number, calendar, slidingrule).getCString();
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}


/* @brief			swig interface for mirGetIMMDate3
*  @param [in]		BaseDate		The base (reference) date
*  @param [in]		Number			The (number-1)th IMM date from the base date, starting from the one closest to the base date
*  @param [in]		Calendar		Calendar
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date from the base date, starting from the one closest to the base date
*/
std::string mirGetIMMDate3(const std::string& BaseDate, 
							int Number, 
							const std::string& Calendar, 
							const std::string& SlidingRule)
{
	std::string ret;
	try 
	{
		LADate baseDate(BaseDate.c_str(),"YYYYMMDD");

		LAString calendar(Calendar.c_str());
		if( calendar == LAString("") ) 
		{
			calendar = LAString("TKB:LNB");
		}

		LAString slidingrule(SlidingRule.c_str());
		if( slidingrule == LAString("") ) 
		{
			slidingrule=LAString("FOLLOWING");
		}

		ret = validation::tryMirGetIMMDate3(baseDate, Number, calendar, slidingrule).getCString();
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;
}