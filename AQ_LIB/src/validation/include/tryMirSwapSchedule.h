#pragma once

#include "AQLCoreTemplateType.h"

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
    AQLStringMatrix tryMirSwapSchedule( const AQLString& accrualStartDate,
                                     const AQLString& accrualEndDateOrTenor,
                                     const AQLString& accrualDaycount,
                                     const AQLString& accrualFrequency,
                                     const AQLString& accrualSlidingRule,
                                     const AQLString& accrualCalendar,
                                     const AQLString& paymentFrequency,
                                     const AQLString& paymentSlidingRule,
                                     const AQLString& paymentCalendar,
                                     const AQLString& paymentLag,
                                     const AQLString& fixingSlidingRule,
                                     const AQLString& fixingCalendar,
                                     const AQLString& fixingLag,
                                     const AQLString& fixingAdvanceOrArrears,
                                     const AQLString& firstStub,
                                     const AQLString& lastStub,
                                     const AQLString& paymentRollDay,
                                     bool showColumnHeaders = true );



}
