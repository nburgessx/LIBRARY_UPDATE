// SwapSchedule.cpp

// Includes: This Library
#include "SwapSchedule.h"
#include "LADateScheduleHelpers.h"
#include "LAMathBaseFuncUtility.h"
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceCFGenUtility.h"
#include "LADateHelpers.h"
#include "LADataReference.h"
#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LADataProcedure.h"
#include "LACoreComponentManager.h"
#include "LAPriceDataConvention.h"
#include "LADataMultiReference.h"

// Includes: Standard Library
#include <cmath>
#include <map>

// Namespaces
using namespace std;


namespace etrading
{
    SwapSchedule::SwapSchedule( const LADate&       accrualStartDate,
                                const LADate&       accrualEndDate,
                                const LAString&     accrualDaycount,
                                const LAString&     accrualFrequency,
                                const LAString&     accrualRollConvention,      // aka Accrual Sliding Rule
                                const LAString&     accrualCalendar,
                                const LAString&     paymentFrequency,
                                const LAString&     paymentRollConvention,      // aka Payment Sliding Rule
                                const LAString&     paymentCalendar,
                                const LAString&     paymentLag,
                                const LAString&     fixingLag,
                                const LAString&     fixingRollConvention,
                                const LAString&     fixingCalendar,
                                const LADate&       firstStub,                  // aka First Odd Date
                                const LADate&       lastStub,                   // aka Last Odd Date
                                const LAString&     rollDayOrConvention,        // Roll Day ( e.g. 9th of month ) or Roll Convention ( IMM, ECM, EOM, End of Month ... )
                                const bool&         isStartRoll,
                                const bool&         isFixingInAdvance )         // Fixing in Advance ( True ) or Arrears ( False )
    {
        // Date Validation
        if( !LADateScheduleHelpers::isValidDate( accrualStartDate ) )
        {
            throw LACoreInvalidData( "#Error: Invalid 'AccrualStartDate'.", __FILE__, __LINE__ );
        }

        if( !LADateScheduleHelpers::isValidDate( accrualEndDate ) )
        {
            throw LACoreInvalidData( "#Error: Invalid 'AccrualEndDate'.", __FILE__, __LINE__ );
        }

        if ( accrualStartDate >= accrualEndDate )
        {
            throw LACoreInvalidData( "#Error: The 'AccrualStartDate' must be strictly greater than the 'AccrualEndDate'.", __FILE__, __LINE__ );
        }

        DateVector      accrualBaseDates;
        LAString        accFrequency            = accrualFrequency;
        LAString        accRollConvention       = accrualRollConvention;
        LAString        accCalendar             = accrualCalendar;

        const LADate*   pFirstStub              = ( firstStub == LADate() ) ? NULL : pFirstStub = & firstStub;
        const LADate*   pLastStub               = ( lastStub  == LADate() ) ? NULL : pLastStub  = & lastStub;

        if ( pFirstStub != NULL )
        {
            if( !LADateScheduleHelpers::isValidDate( firstStub ) )
            {
                throw LACoreInvalidData( "#Error: Invalid 'FirstStub'.", __FILE__, __LINE__ );
            }
        }

        if ( pLastStub != NULL )
        {
            if( !LADateScheduleHelpers::isValidDate( lastStub ) )
            {
                throw LACoreInvalidData( "#Error: Invalid 'LastStub'.", __FILE__, __LINE__ );
            }
        }

        if ( pFirstStub != NULL && firstStub < accrualStartDate )
        {
            throw LACoreInvalidData( "#Error: The 'FirstStub' cannot be before the 'AccrualStartDate'.", __FILE__, __LINE__ );
        }

        if ( pLastStub != NULL && lastStub > accrualEndDate )
        {
            throw LACoreInvalidData( "#Error: The 'LastStub' cannot be after the 'AccrualEndDate'.", __FILE__, __LINE__ );
        }

        if ( pFirstStub != NULL && pLastStub != NULL && firstStub > lastStub )
        {
            throw LACoreInvalidData( "#Error: The 'LastStub' cannot be before the 'FirstStub'.", __FILE__, __LINE__ );
        }


        // Daycount Basis, default is ACT/360 ( This is the floating swap leg convention for JPY, USD, EUR swaps )
        // Make uppercase for validation check below.
        LAString        dayCount                = accrualDaycount;
        dayCount.toUpper();

        if ( dayCount == LAString( "" ) )
        {
            dayCount    = LAString( "ACT/360" );
        }
        else if ( dayCount != LAString( "ACT/360" )      && dayCount != LAString( "30/360" )
                  && dayCount != LAString( "ACT/365" )      && dayCount != LAString( "30E/360" )
                  && dayCount != LAString( "ACT/365_ISDA" ) && dayCount != LAString( "ACT/365FJ" ) )
        {
            throw LACoreInvalidData( "#Error: Invalid 'AccrualDaycount', must be 'Act/360', 'Act/365', 'Act/365_ISDA', 'Act/365FJ', '30/360', '30E/360' or left blank.", __FILE__, __LINE__ );
        }

        // Roll Day or Roll Convention
        // ---------------------------

        // This can be an integer representing a day of the month or can be a string to represent a convention
        // such as e.g. IMM, ECB, EOM, ... et al. For example 9 would mean roll on the 9th of the month and IMM
        // would mean roll on the IMM date

        int tempRollDay;
        LAString tempRollDayConvention;

        int*            pRollDay                = NULL;
        LAString*       pRollDayConvention      = NULL;

        if ( rollDayOrConvention != LAString( "" ) )
        {
            // Quick and Dirty Check if rollDayOrConvention is a Roll Day Convention or a day of the month
            if ( rollDayOrConvention == LAString( "EOM" )
                    || rollDayOrConvention == LAString( "IMM" ) )
            {
                tempRollDayConvention           = rollDayOrConvention;
                pRollDayConvention              = & tempRollDayConvention;
            }
            else if ( rollDayOrConvention == LAString( "1" )    || rollDayOrConvention == LAString( "17" )
                      || rollDayOrConvention == LAString( "2" )    || rollDayOrConvention == LAString( "18" )
                      || rollDayOrConvention == LAString( "3" )    || rollDayOrConvention == LAString( "19" )
                      || rollDayOrConvention == LAString( "4" )    || rollDayOrConvention == LAString( "20" )
                      || rollDayOrConvention == LAString( "5" )    || rollDayOrConvention == LAString( "21" )
                      || rollDayOrConvention == LAString( "6" )    || rollDayOrConvention == LAString( "22" )
                      || rollDayOrConvention == LAString( "7" )    || rollDayOrConvention == LAString( "23" )
                      || rollDayOrConvention == LAString( "8" )    || rollDayOrConvention == LAString( "24" )
                      || rollDayOrConvention == LAString( "9" )    || rollDayOrConvention == LAString( "25" )
                      || rollDayOrConvention == LAString( "10" )   || rollDayOrConvention == LAString( "26" )
                      || rollDayOrConvention == LAString( "11" )   || rollDayOrConvention == LAString( "27" )
                      || rollDayOrConvention == LAString( "12" )   || rollDayOrConvention == LAString( "28" )
                      || rollDayOrConvention == LAString( "13" )   || rollDayOrConvention == LAString( "29" )
                      || rollDayOrConvention == LAString( "14" )   || rollDayOrConvention == LAString( "30" )
                      || rollDayOrConvention == LAString( "15" )   || rollDayOrConvention == LAString( "31" )
                      || rollDayOrConvention == LAString( "16" )   )
            {
                tempRollDay                     = rollDayOrConvention.getIntValue();
                pRollDay                        = & tempRollDay;
            }
            else
            {
                throw LACoreInvalidData( "#Error: 'RollDayOrConvention' must be a day of the month or a date convention i.e. IMM or EOM (End of Month).", __FILE__, __LINE__ );
            }
        }

        // Generate the Accrual Base Dates: This is a one dimensional vector representing accrual start and end dates
        accrualBaseDates = LADateScheduleHelpers::generateSchedule( accrualStartDate,
                           accrualEndDate,
                           accFrequency,
                           accRollConvention,    // aka Sliding Rule
                           accCalendar,
                           pFirstStub,           // aka First Odd Date
                           pLastStub,            // aka Last Odd Date
                           pRollDay,
                           isStartRoll,
                           pRollDayConvention );

        // We need at least 2 dates generated by the accrual and payment date generators i.e. a start and end date.
        size_t MINIMUM_NUMBER_OF_DATES = 2;

        if ( accrualBaseDates.empty() || accrualBaseDates.size() < MINIMUM_NUMBER_OF_DATES )
        {
            throw LACoreInvalidData( "#Error: Unable to generate accrual dates with the parameters specified.", __FILE__, __LINE__ );
        }

        DateVector  paymentBaseDates;
        LAString    payFrequency        = paymentFrequency;
        LAString    payRollConvention   = paymentRollConvention;
        LAString    payCalendar         = paymentCalendar;

        // Generate the Payment Base Dates: This is a one dimensional vector representing payment start and end dates
        paymentBaseDates = LADateScheduleHelpers::generateSchedule( accrualStartDate,
																	accrualEndDate,
																	payFrequency,
																	payRollConvention,
																	payCalendar,
																	pFirstStub,
																	pLastStub,
																	pRollDay,
																	isStartRoll,
																	pRollDayConvention );

        if ( paymentBaseDates.empty() || paymentBaseDates.size() < MINIMUM_NUMBER_OF_DATES )
        {
            throw LACoreInvalidData( "#Error: Unable to generate the payment dates using the parameters specified.", __FILE__, __LINE__ );
        }

        // Schedule Data Placeholders
        DoubleMatrix    scheduleResults;
        DoubleVector    scheduleRow;

        LADate      thisStartDate           = LADate();
        LADate      thisEndDate             = LADate();
        LADate      thisFixingDateNoLag     = LADate();
        LADate      thisFixingDateWithLag   = LADate();
        LADate      thisPaymentDateNoLag    = LADate();
        LADate      thisPaymentDateWithLag  = LADate();
        double      thisAccrualPeriod       = 0.0;

        LADate      lastEndDate             = LADate();

        // Accrual and Payment Position Indicies
        // -------------------------------------
        // The 'accrualRowIndex' and 'paymentRowIndex' control the track the current accrual period
        // and payment period we are working on. This is needed since accrued coupons can be divided
        // into multiple payments or several coupons combined into a single payment.
        size_t      accrualRowIndex         = 0;
        size_t      paymentRowIndex         = 0;

        if (  paymentBaseDates.size() >= accrualBaseDates.size() )
        {
            // Non-Compounding Type Schedule
            // -----------------------------

            // Non-Compounding Type Schedule i.e. we pay more frequently than we accrue coupon interest.
            // This means a single coupon paid in several installments ( Single Shared Fixing Date per Cashflow Payment )

            size_t numberOfRows             = paymentBaseDates.size(); // we already checked size > 0 above
            DateVector::const_iterator it   = paymentBaseDates.begin();

            for( it = paymentBaseDates.begin(); it != paymentBaseDates.end(); ++it )
            {
                // Skip the first payment date ... The first payment date is set-up to be the start date.
                // This is to allow for start date payments e.g. notionalExchange exchanges and upfront fees.
                if ( paymentRowIndex == 0 )
                {
                    // The end date is required since it is the start of the next period.
                    thisEndDate           = accrualBaseDates[ accrualRowIndex ];

                    accrualRowIndex++;
                    paymentRowIndex++;
                }
                else
                {
                    if ( accrualRowIndex == 0 || accrualRowIndex > accrualBaseDates.size() )
                    {
                        throw LACoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
                    }

                    if ( paymentRowIndex == 0 || paymentRowIndex > paymentBaseDates.size() )
                    {
                        throw LACoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
                    }

                    // Clear the existing schedule row
                    scheduleRow.clear();

                    thisPaymentDateNoLag        = *it;
                    thisStartDate               = lastEndDate;

                    //
                    // Update Fixing Rate for New Coupon. Must also check if fixing dates set in advance or arrears
                    //

                    // Since we are splitting a single coupon into multiple payments, we must reuse the same single fixing
                    // rate for each of these payments and only update the fixing, when we are processing the next coupon.
                    // This only works if we are fixing in advance and not in arrears.

                    if ( !isFixingInAdvance && paymentBaseDates.size() > accrualBaseDates.size() )
                    {
                        throw LACoreInvalidData( "#Error: Unable to fix in Arrears, since floating paymennts cannot be made before their fixing dates.", __FILE__, __LINE__ );
                    }

                    if ( ( accrualRowIndex - 1 ) < 0 )
                    {
                        throw LACoreInvalidData( "#Error: Unable to generate fixing dates. Access Violation Error.", __FILE__, __LINE__ );
                    }

                    thisFixingDateNoLag         = accrualBaseDates[ accrualRowIndex - 1 ]; // we check for access violation above

                    // If a payment date is before the coupon end date then set the accrual end date to the payment date.
                    // This will have the effect of dividing up the coupon by the number of payment dates.
                    if ( thisPaymentDateNoLag < accrualBaseDates[ accrualRowIndex ]  )
                    {
                        thisEndDate             = thisPaymentDateNoLag;
                    }
                    else
                    {
                        thisEndDate             = accrualBaseDates[ accrualRowIndex ];
                        accrualRowIndex++;
                    }

                    // Apply Payment Lag
                    thisFixingDateWithLag       = LADateScheduleHelpers::getDate( isFixingInAdvance ? thisFixingDateNoLag : thisEndDate,
                                                  fixingLag,
                                                  fixingRollConvention,
                                                  fixingCalendar );

                    thisPaymentDateWithLag      = LADateScheduleHelpers::getDate( thisPaymentDateNoLag,
                                                  paymentLag,
                                                  paymentRollConvention,
                                                  paymentCalendar );


                    // Accrual Period or Year Fraction
                    // Important Note: If compound interest is being applied then we accrue interest to the
                    // payment date ( with no payment lag ) and not the accrual end date.
                    bool DONT_INCLUDE_LAST_PAYMENT_DATE = false;
                    thisAccrualPeriod           = LADateScheduleHelpers::getTerm( thisStartDate,
                                                  thisEndDate,
                                                  dayCount,
                                                  DONT_INCLUDE_LAST_PAYMENT_DATE );

                    if ( thisFixingDateWithLag > thisPaymentDateWithLag )
                    {
                        throw LACoreInvalidData( "#Error: Payments cannot be made before fixing dates. Please check if the schedule payment and fixing lags and other inputs are correct.", __FILE__, __LINE__ );
                    }

                    if ( thisStartDate > thisEndDate )
                    {
                        throw LACoreInvalidData( "#Error: Accrual start dates cannot be after the accrual end dates. Please check that the schedule inputs are correct.", __FILE__, __LINE__ );
                    }

                    // Update the schedule's current row of results
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisFixingDateWithLag ) ) );
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisStartDate ) ) );
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisEndDate ) ) );
                    scheduleRow.push_back( thisAccrualPeriod );
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisPaymentDateWithLag ) ) );

                    // Update the scheduleResults matrix
                    scheduleResults.push_back( scheduleRow );

                    // Update the Position Indicies: We are iterating over the larger date set i.e. the payment dates
                    paymentRowIndex++;
                }

                // Update Last Period Dates
                lastEndDate         = thisEndDate;
            }

        }
        else
        {
            // Compounding Type Schedule
            // -----------------------------

            // Compounding Type Schedule i.e. we pay several coupons in a single payment ( Multiple Fixings )
            // This means multiple coupons are being paid in a single installment, whereby Coupons are deferred
            // until the next payment date.As such any deferred coupons would accumulate compound interest.

            size_t numberOfRows             = accrualBaseDates.size(); // we already checked size > 0 above
            DateVector::const_iterator it   = accrualBaseDates.begin();

            for( it = accrualBaseDates.begin(); it != accrualBaseDates.end(); ++it )
            {
                // Skip the first accrual end date ... The first payment date is set-up to be the start date.
                // This is to allow for start date payments e.g. notionalExchange exchanges and upfront fees.
                if ( accrualRowIndex == 0 )
                {
                    // The end date is required since it is the start of the next period.
                    thisEndDate             = accrualBaseDates[ accrualRowIndex ];

                    accrualRowIndex++;
                    paymentRowIndex++;
                }
                else
                {
                    if ( accrualRowIndex == 0 || accrualRowIndex > accrualBaseDates.size() )
                    {
                        throw LACoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
                    }

                    if ( paymentRowIndex == 0 || paymentRowIndex > paymentBaseDates.size() )
                    {
                        throw LACoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
                    }

                    // Clear the existing schedule row
                    scheduleRow.clear();

                    thisStartDate               = lastEndDate;
                    thisEndDate                 = *it;
                    thisPaymentDateNoLag        = paymentBaseDates[ paymentRowIndex ];

                    // Keep the payment date fixed until all coupons ending before the payment date have been paid. This will
                    // have the effect of accumulating and bucketing all coupons into the next available payment date.
                    if( thisEndDate >=  paymentBaseDates[ paymentRowIndex ] )
                    {
                        paymentRowIndex++;
                    }

                    // Must check if fixing dates set in advance or arrears
                    thisFixingDateWithLag       = LADateScheduleHelpers::getDate( isFixingInAdvance ? thisStartDate : thisEndDate,
                                                  fixingLag,
                                                  fixingRollConvention,
                                                  fixingCalendar );

                    thisPaymentDateWithLag      = LADateScheduleHelpers::getDate( thisPaymentDateNoLag,
                                                  paymentLag,
                                                  paymentRollConvention,
                                                  paymentCalendar );


                    // Accrual Period or Year Fraction
                    // Important Note: If compound interest is being applied then we accrue interest to the
                    // payment date ( with no payment lag ) and not the accrual end date.
                    bool DONT_INCLUDE_LAST_PAYMENT_DATE = false;
                    thisAccrualPeriod           = LADateScheduleHelpers::getTerm( thisStartDate,
                                                  thisEndDate,
                                                  dayCount,
                                                  DONT_INCLUDE_LAST_PAYMENT_DATE );

                    if ( thisFixingDateWithLag > thisPaymentDateWithLag )
                    {
                        throw LACoreInvalidData( "#Error: Payments cannot be made before fixing dates. Please check if the schedule payment and fixing lags and other inputs are correct.", __FILE__, __LINE__ );
                    }

                    if ( thisStartDate > thisEndDate )
                    {
                        throw LACoreInvalidData( "#Error: Accrual start dates cannot be after the accrual end dates. Please check that the schedule inputs are correct.", __FILE__, __LINE__ );
                    }

                    // Update the schedule's current row of results
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisFixingDateWithLag ) ) );
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisStartDate ) ) );
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisEndDate ) ) );
                    scheduleRow.push_back( thisAccrualPeriod );
                    scheduleRow.push_back( ( double )( LADateScheduleHelpers::getExcelDate( thisPaymentDateWithLag ) ) );

                    // Update the scheduleResults matrix
                    scheduleResults.push_back( scheduleRow );

                    // Update the Position Indices: We are iterating over the larger date set i.e. the accrual dates
                    accrualRowIndex++;
                }

                // Update Last Period Dates
                lastEndDate         = thisEndDate;
            }
        }

        schedule_ = scheduleResults;
        return;
    }
}
