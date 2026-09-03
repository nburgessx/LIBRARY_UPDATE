#pragma once

#include "LACoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for the mirSwapSchedule method
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
    LAStringMatrix tryMirSwapSchedule( const LAString& accrualStartDate,
                                     const LAString& accrualEndDateOrTenor,
                                     const LAString& accrualDaycount,
                                     const LAString& accrualFrequency,
                                     const LAString& accrualSlidingRule,
                                     const LAString& accrualCalendar,
                                     const LAString& paymentFrequency,
                                     const LAString& paymentSlidingRule,
                                     const LAString& paymentCalendar,
                                     const LAString& paymentLag,
                                     const LAString& fixingSlidingRule,
                                     const LAString& fixingCalendar,
                                     const LAString& fixingLag,
                                     const LAString& fixingAdvanceOrArrears,
                                     const LAString& firstStub,
                                     const LAString& lastStub,
                                     const LAString& paymentRollDay,
                                     bool showColumnHeaders = true );



}
