#pragma once

// Includes: This Library
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "ConstantDeclarations.h"
#include "LATime.h"

// Forward Declarations
class AQLPriceDataSlidingRule;
class AQLPriceDataCalendar;

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
        SwapSchedule( const AQLDate&       accrualStartDate,
                      const AQLDate&       accrualEndDate,
                      const AQLString&     accrualDaycount,
                      const AQLString&     accrualFrequency,
                      const AQLString&     accrualRollConvention,            // aka Accrual Sliding Rule
                      const AQLString&     accrualCalendar,
                      const AQLString&     paymentFrequency,
                      const AQLString&     paymentRollConvention,           // aka Payment Sliding Rule
                      const AQLString&     paymentCalendar,
                      const AQLString&     paymentLag,
                      const AQLString&     fixingLag,
                      const AQLString&     fixingRollConvention,
                      const AQLString&     fixingCalendar,
                      const AQLDate&       firstStub,                       // aka First Odd Date
                      const AQLDate&       lastStub,                        // aka Last Odd Date
                      const AQLString&     rollDayOrConvention,             // Roll Day ( e.g. 9th of month ) or Roll Convention ( IMM, ECM, EOM, End of Month ... )
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

