
#include "AQObjCurveDayAdjustment.h"
#include "ContainerUtilities.h"


namespace etrading
{
    // TODO: move to delegating CTORs for C++11 ...
    AQObjCurveDayAdjustment::AQObjCurveDayAdjustment(	const BusinessDayAdjustmentEnum businessDayAdjustment,
            const std::string& dateCalendar )
        :	fixingDateBusinessDayAdjustment_( businessDayAdjustment ),
          fixingDateCalendar_( trim_to_upper( dateCalendar.c_str() ) ),
          accrualEndDateDateBusinessDayAdjustment_( businessDayAdjustment ),
          accrualEndDateDateCalendar_( trim_to_upper( dateCalendar.c_str() ) ),
          paymentDateBusinessDayAdjustment_( businessDayAdjustment ),
          paymentDateCalendar_( trim_to_upper( dateCalendar.c_str() ) )
    {
        aqFixingDateCalendar_ = &AQLCalendarSet::getCalendar( fixingDateCalendar_.c_str() );
        aqAccrualEndDateDateCalendar_ = &AQLCalendarSet::getCalendar( accrualEndDateDateCalendar_.c_str() );
        aqPaymentDateCalendar_ = &AQLCalendarSet::getCalendar( paymentDateCalendar_.c_str() );
    };

    AQObjCurveDayAdjustment::AQObjCurveDayAdjustment( const BusinessDayAdjustmentEnum fixingDateBusinessDayAdjustment,
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
        aqFixingDateCalendar_ = &AQLCalendarSet::getCalendar( fixingDateCalendar_.c_str() );
        aqAccrualEndDateDateCalendar_ = &AQLCalendarSet::getCalendar( accrualEndDateDateCalendar_.c_str() );
        aqPaymentDateCalendar_ = &AQLCalendarSet::getCalendar( paymentDateCalendar_.c_str() );
    };

}