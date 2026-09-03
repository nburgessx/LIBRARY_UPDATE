#include "InitializeETrading.h"
#include "TypeUtilities.h"
#include "mirSwapSchedule.h"
#include "tryMirSwapSchedule.h"


/* @brief			swig interface for the mirSwapSchedule method
*  @param [in]		accrualStartDate				Accrual start date
*  @param [in]		accrualEndDateOrTenor			Accrual end date or tenor
*  @param [in]		accrualDaycount					Accrual day count convention
*  @param[in]		accrualFrequency				Accrual frequencey
*  @param[in]		accrualSlidingRule				Accrual Roll Convention or Sliding Rule
*  @param[in]		accrualCalendar					Accrual calendar
*  @param[in]		paymentFrequency				Payment frequencey
*  @param[in]		paymentSlidingRule				Payment Roll Convention or Sliding Rule
*  @param[in]		paymentCalendar					Payment calendar
*  @param[in]		paymentLag						Payment Lag
*  @param[in]		fixingSlidingRule				Fixing Roll Convention or Sliding Rule
*  @param[in]		fixingCalendar					Fixing calendar
*  @param[in]		fixingLag						Fixing Lag
*  @param[in]		fixingAdvanceOrArrears	        Fixing in advance or arrears, default to fixing in advance
*  @param[in]		firstStub						First Odd Date, i.e. end date of the front stub period
*  @param[in]		lastStub						Last Odd Date, i.e. start date of the end stub period
*  @param[in]		paymentRollDay					Roll Day of Month (e.g. 9th of month ) 
*  @param[in]		showColumnHeaders				True to show column headers. 
*  @return			a matrix of floading leg/fixing leg schedules 
*/
SWIG_STRINGMATRIX mirSwapSchedule( const std::string& accrualStartDate,
                                   const std::string& accrualEndDateOrTenor,
                                   const std::string& accrualDaycount,
                                   const std::string& accrualFrequency,
						           const std::string& accrualSlidingRule,            
                                   const std::string& accrualCalendar,
                                   const std::string& paymentFrequency,
						           const std::string& paymentSlidingRule,           
                                   const std::string& paymentCalendar,
                                   const std::string& paymentLag,
                                   const std::string& fixingSlidingRule,
                                   const std::string& fixingCalendar,
                                   const std::string& fixingLag,
						           const std::string& fixingAdvanceOrArrears, 
						           const std::string& firstStub,                       
                                   const std::string& lastStub,                        
                                   const std::string& paymentRollDay,
						           bool showColumnHeaders) 
{
	SWIG_STRINGMATRIX ret;
	try 
	{
		// Input marshalling
		AQLString aStartDate		(accrualStartDate.c_str());
		AQLString aEndDateOrTenor(accrualEndDateOrTenor.c_str());
		AQLString aDaycount		(accrualDaycount.c_str());
		AQLString aFrequency		(accrualFrequency.c_str());
		AQLString aSRule			(accrualSlidingRule.c_str());
		AQLString aCalendar		(accrualCalendar.c_str());
		AQLString pFrequency		(paymentFrequency.c_str());
		AQLString pSRule			(paymentSlidingRule.c_str());
		AQLString pCalendar		(paymentCalendar.c_str());
		AQLString pLag			(paymentLag.c_str());
		AQLString fixingSRule	(fixingSlidingRule.c_str());
		AQLString fixingCal		(fixingCalendar.c_str());
		AQLString fixingLg		(fixingLag.c_str());
		AQLString fixingAdAr		(fixingAdvanceOrArrears.c_str());
		AQLString firstSt		(firstStub.c_str());
		AQLString lastSt			(lastStub.c_str());
		AQLString pRollDay		(paymentRollDay.c_str());

		AQLStringMatrix temp = validation::tryMirSwapSchedule(
										aStartDate, aEndDateOrTenor, aDaycount, aFrequency, aSRule, aCalendar,
										pFrequency, pSRule, pCalendar, pLag,
										fixingSRule, fixingCal, fixingLg, fixingAdAr,
										firstSt, lastSt, pRollDay);

		ret = swig::fromStringMatrixToMatrixOfString(temp);
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}