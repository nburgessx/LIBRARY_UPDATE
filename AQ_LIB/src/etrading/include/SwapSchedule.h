#pragma once

// Includes: This Library
#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"
#include "ConstantDeclarations.h"
#include "LATime.h"

// Forward Declarations
class LAPriceDataSlidingRule;
class LAPriceDataCalendar;

// Namespaces
using namespace std;

namespace etrading
{
    class SwapSchedule
    {
    public:

        SwapSchedule() {};
        virtual ~SwapSchedule() {};

        // Alternative Constructor
        SwapSchedule( const LADate&       accrualStartDate,
                      const LADate&       accrualEndDate,
                      const LAString&     accrualDaycount,
                      const LAString&     accrualFrequency,
                      const LAString&     accrualRollConvention,            // aka Accrual Sliding Rule
                      const LAString&     accrualCalendar,
                      const LAString&     paymentFrequency,
                      const LAString&     paymentRollConvention,           // aka Payment Sliding Rule
                      const LAString&     paymentCalendar,
                      const LAString&     paymentLag,
                      const LAString&     fixingLag,
                      const LAString&     fixingRollConvention,
                      const LAString&     fixingCalendar,
                      const LADate&       firstStub,                       // aka First Odd Date
                      const LADate&       lastStub,                        // aka Last Odd Date
                      const LAString&     rollDayOrConvention,             // Roll Day ( e.g. 9th of month ) or Roll Convention ( IMM, ECM, EOM, End of Month ... )
                      const bool&         isStartRoll = true,
                      const bool&         isFixingInAdvance = true );      // Fixing in Advance ( True ) or Arrears ( False )

        DoubleMatrix getSchedule()
        {
            return schedule_;
        }


    private:
        DoubleMatrix schedule_;

    };
}

