#include "InitializeMLibETrading.h"
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
		LAString aStartDate		(accrualStartDate.c_str());
		LAString aEndDateOrTenor(accrualEndDateOrTenor.c_str());
		LAString aDaycount		(accrualDaycount.c_str());
		LAString aFrequency		(accrualFrequency.c_str());
		LAString aSRule			(accrualSlidingRule.c_str());
		LAString aCalendar		(accrualCalendar.c_str());
		LAString pFrequency		(paymentFrequency.c_str());
		LAString pSRule			(paymentSlidingRule.c_str());
		LAString pCalendar		(paymentCalendar.c_str());
		LAString pLag			(paymentLag.c_str());
		LAString fixingSRule	(fixingSlidingRule.c_str());
		LAString fixingCal		(fixingCalendar.c_str());
		LAString fixingLg		(fixingLag.c_str());
		LAString fixingAdAr		(fixingAdvanceOrArrears.c_str());
		LAString firstSt		(firstStub.c_str());
		LAString lastSt			(lastStub.c_str());
		LAString pRollDay		(paymentRollDay.c_str());

		LAStringMatrix temp = validation_api::tryMirSwapSchedule(
										aStartDate, aEndDateOrTenor, aDaycount, aFrequency, aSRule, aCalendar,
										pFrequency, pSRule, pCalendar, pLag,
										fixingSRule, fixingCal, fixingLg, fixingAdAr,
										firstSt, lastSt, pRollDay);

		ret = swig::fromStringMatrixToMatrixOfString(temp);
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}