#pragma once

#include <vector>
#include <string>

#include "CoreEnumerations.h"
#include "AQLMathCalendarSet.h"
#include "AQLMathCalendar.h"

/*
This class brings togetther the fixing business day adjusment, the accrual day business adjustment and the payment day adjustment.
At the moment the LWOCurve object does not use the accrual day business adjusment
*/


namespace etrading
{

    class LWOCurveDayAdjustment
    {
    public:
        LWOCurveDayAdjustment( const BusinessDayAdjustmentEnum businessDayAdjustment,
                               const std::string& dateCalendar );
        LWOCurveDayAdjustment( const BusinessDayAdjustmentEnum fixingDateBusinessDayAdjustment,
                               const std::string& fixingDateCalendar,
                               const BusinessDayAdjustmentEnum accrualEndDateDateBusinessDayAdjustment,
                               const std::string& accrualEndDateDateCalendar,
                               const BusinessDayAdjustmentEnum paymentDateBusinessDayAdjustment,
                               const std::string& paymentDateCalendar );
    private:
        // this adjusts from fixing date to the Accrual Start Date
        const BusinessDayAdjustmentEnum fixingDateBusinessDayAdjustment_;
        const std::string& fixingDateCalendar_;
        const AQLMathCalendar* mlibFixingDateCalendar_;

        const BusinessDayAdjustmentEnum accrualEndDateDateBusinessDayAdjustment_;
        const std::string& accrualEndDateDateCalendar_;
        const AQLMathCalendar* mlibAccrualEndDateDateCalendar_;

        const BusinessDayAdjustmentEnum paymentDateBusinessDayAdjustment_;
        const std::string& paymentDateCalendar_;
        const AQLMathCalendar* mlibPaymentDateCalendar_;
    };

};