
#include "AQOCurveDayAdjustment.h"
#include "ContainerUtilities.h"


namespace etrading
{
    // TODO: move to delegating CTORs for C++11 ...
    AQOCurveDayAdjustment::AQOCurveDayAdjustment(	const BusinessDayAdjustmentEnum businessDayAdjustment,
            const std::string& dateCalendar )
        :	fixingDateBusinessDayAdjustment_( businessDayAdjustment ),
          fixingDateCalendar_( trim_to_upper( dateCalendar.c_str() ) ),
          accrualEndDateDateBusinessDayAdjustment_( businessDayAdjustment ),
          accrualEndDateDateCalendar_( trim_to_upper( dateCalendar.c_str() ) ),
          paymentDateBusinessDayAdjustment_( businessDayAdjustment ),
          paymentDateCalendar_( trim_to_upper( dateCalendar.c_str() ) )
    {
        mlibFixingDateCalendar_ = &AQLMathCalendarSet::getCalendar( fixingDateCalendar_.c_str() );
        mlibAccrualEndDateDateCalendar_ = &AQLMathCalendarSet::getCalendar( accrualEndDateDateCalendar_.c_str() );
        mlibPaymentDateCalendar_ = &AQLMathCalendarSet::getCalendar( paymentDateCalendar_.c_str() );
    };

    AQOCurveDayAdjustment::AQOCurveDayAdjustment( const BusinessDayAdjustmentEnum fixingDateBusinessDayAdjustment,
            const std::string& fixingDateCalendar,
            const BusinessDayAdjustmentEnum accrualEndDateDateBusinessDayAdjustment,
            const std::string& accrualEndDateDateCalendar,
            const BusinessDayAdjustmentEnum paymentDateBusinessDayAdjustment,
            const std::string& paymentDateCalendar )
        : 	fixingDateBusinessDayAdjustment_( fixingDateBusinessDayAdjustment ),
           fixingDateCalendar_( trim_to_upper( fixingDateCalendar.c_str() ) ),
           accrualEndDateDateBusinessDayAdjustment_( accrualEndDateDateBusinessDayAdjustment ),
           accrualEndDateDateCalendar_( trim_to_upper( accrualEndDateDateCalendar.c_str() ) ),
           paymentDateBusinessDayAdjustment_( paymentDateBusinessDayAdjustment ),
           paymentDateCalendar_( trim_to_upper( paymentDateCalendar.c_str() ) )
    {
        mlibFixingDateCalendar_ = &AQLMathCalendarSet::getCalendar( fixingDateCalendar_.c_str() );
        mlibAccrualEndDateDateCalendar_ = &AQLMathCalendarSet::getCalendar( accrualEndDateDateCalendar_.c_str() );
        mlibPaymentDateCalendar_ = &AQLMathCalendarSet::getCalendar( paymentDateCalendar_.c_str() );
    };

}