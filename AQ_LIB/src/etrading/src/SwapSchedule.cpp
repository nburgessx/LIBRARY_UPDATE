// SwapSchedule.cpp

// Includes: This Library
#include "SwapSchedule.h"
#include "LADateScheduleHelpers.h"
#include "LAMathBaseFuncUtility.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLPriceDataDayCount.h"
#include "LAPriceCFGenUtility.h"
#include "LADateHelpers.h"
#include "AQLDataReference.h"
#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "AQLDataProcedure.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataMultiReference.h"

// Includes: Standard Library
#include <cmath>
#include <map>

// Namespaces
using namespace std;


namespace etrading
{
    SwapSchedule::SwapSchedule( const AQLDate&       accrualStartDate,
                                const AQLDate&       accrualEndDate,
                                const AQLString&     accrualDaycount,
                                const AQLString&     accrualFrequency,
                                const AQLString&     accrualRollConvention,      // aka Accrual Sliding Rule
                                const AQLString&     accrualCalendar,
                                const AQLString&     paymentFrequency,
                                const AQLString&     paymentRollConvention,      // aka Payment Sliding Rule
                                const AQLString&     paymentCalendar,
                                const AQLString&     paymentLag,
                                const AQLString&     fixingLag,
                                const AQLString&     fixingRollConvention,
                                const AQLString&     fixingCalendar,
                                const AQLDate&       firstStub,                  // aka First Odd Date
                                const AQLDate&       lastStub,                   // aka Last Odd Date
                                const AQLString&     rollDayOrConvention,        // Roll Day ( e.g. 9th of month ) or Roll Convention ( IMM, ECM, EOM, End of Month ... )
                                const bool&         isStartRoll,
                                const bool&         isFixingInAdvance )         // Fixing in Advance ( True ) or Arrears ( False )
    {
        // Date Validation
        if( !LADateScheduleHelpers::isValidDate( accrualStartDate ) )
        {
            throw AQLCoreInvalidData( "#Error: Invalid 'AccrualStartDate'.", __FILE__, __LINE__ );
        }

        if( !LADateScheduleHelpers::isValidDate( accrualEndDate ) )
        {
            throw AQLCoreInvalidData( "#Error: Invalid 'AccrualEndDate'.", __FILE__, __LINE__ );
        }

        if ( accrualStartDate >= accrualEndDate )
        {
            throw AQLCoreInvalidData( "#Error: The 'AccrualStartDate' must be strictly greater than the 'AccrualEndDate'.", __FILE__, __LINE__ );
        }

        DateVector      accrualBaseDates;
        AQLString        accFrequency            = accrualFrequency;
        AQLString        accRollConvention       = accrualRollConvention;
        AQLString        accCalendar             = accrualCalendar;

        const AQLDate*   pFirstStub              = ( firstStub == AQLDate() ) ? NULL : pFirstStub = & firstStub;
        const AQLDate*   pLastStub               = ( lastStub  == AQLDate() ) ? NULL : pLastStub  = & lastStub;

        if ( pFirstStub != NULL )
        {
            if( !LADateScheduleHelpers::isValidDate( firstStub ) )
            {
                throw AQLCoreInvalidData( "#Error: Invalid 'FirstStub'.", __FILE__, __LINE__ );
            }
        }

        if ( pLastStub != NULL )
        {
            if( !LADateScheduleHelpers::isValidDate( lastStub ) )
            {
                throw AQLCoreInvalidData( "#Error: Invalid 'LastStub'.", __FILE__, __LINE__ );
            }
        }

        if ( pFirstStub != NULL && firstStub < accrualStartDate )
        {
            throw AQLCoreInvalidData( "#Error: The 'FirstStub' cannot be before the 'AccrualStartDate'.", __FILE__, __LINE__ );
        }

        if ( pLastStub != NULL && lastStub > accrualEndDate )
        {
            throw AQLCoreInvalidData( "#Error: The 'LastStub' cannot be after the 'AccrualEndDate'.", __FILE__, __LINE__ );
        }

        if ( pFirstStub != NULL && pLastStub != NULL && firstStub > lastStub )
        {
            throw AQLCoreInvalidData( "#Error: The 'LastStub' cannot be before the 'FirstStub'.", __FILE__, __LINE__ );
        }


        // Daycount Basis, default is ACT/360 ( This is the floating swap leg convention for JPY, USD, EUR swaps )
        // Make uppercase for validation check below.
        AQLString        dayCount                = accrualDaycount;
        dayCount.toUpper();

        if ( dayCount == AQLString( "" ) )
        {
            dayCount    = AQLString( "ACT/360" );
        }
        else if ( dayCount != AQLString( "ACT/360" )      && dayCount != AQLString( "30/360" )
                  && dayCount != AQLString( "ACT/365" )      && dayCount != AQLString( "30E/360" )
                  && dayCount != AQLString( "ACT/365_ISDA" ) && dayCount != AQLString( "ACT/365FJ" ) )
        {
            throw AQLCoreInvalidData( "#Error: Invalid 'AccrualDaycount', must be 'Act/360', 'Act/365', 'Act/365_ISDA', 'Act/365FJ', '30/360', '30E/360' or left blank.", __FILE__, __LINE__ );
        }

        // Roll Day or Roll Convention
        // ---------------------------

        // This can be an integer representing a day of the month or can be a string to represent a convention
        // such as e.g. IMM, ECB, EOM, ... et al. For example 9 would mean roll on the 9th of the month and IMM
        // would mean roll on the IMM date

        int tempRollDay;
        AQLString tempRollDayConvention;

        int*            pRollDay                = NULL;
        AQLString*       pRollDayConvention      = NULL;

        if ( rollDayOrConvention != AQLString( "" ) )
        {
            // Quick and Dirty Check if rollDayOrConvention is a Roll Day Convention or a day of the month
            if ( rollDayOrConvention == AQLString( "EOM" )
                    || rollDayOrConvention == AQLString( "IMM" ) )
            {
                tempRollDayConvention           = rollDayOrConvention;
                pRollDayConvention              = & tempRollDayConvention;
            }
            else if ( rollDayOrConvention == AQLString( "1" )    || rollDayOrConvention == AQLString( "17" )
                      || rollDayOrConvention == AQLString( "2" )    || rollDayOrConvention == AQLString( "18" )
                      || rollDayOrConvention == AQLString( "3" )    || rollDayOrConvention == AQLString( "19" )
                      || rollDayOrConvention == AQLString( "4" )    || rollDayOrConvention == AQLString( "20" )
                      || rollDayOrConvention == AQLString( "5" )    || rollDayOrConvention == AQLString( "21" )
                      || rollDayOrConvention == AQLString( "6" )    || rollDayOrConvention == AQLString( "22" )
                      || rollDayOrConvention == AQLString( "7" )    || rollDayOrConvention == AQLString( "23" )
                      || rollDayOrConvention == AQLString( "8" )    || rollDayOrConvention == AQLString( "24" )
                      || rollDayOrConvention == AQLString( "9" )    || rollDayOrConvention == AQLString( "25" )
                      || rollDayOrConvention == AQLString( "10" )   || rollDayOrConvention == AQLString( "26" )
                      || rollDayOrConvention == AQLString( "11" )   || rollDayOrConvention == AQLString( "27" )
                      || rollDayOrConvention == AQLString( "12" )   || rollDayOrConvention == AQLString( "28" )
                      || rollDayOrConvention == AQLString( "13" )   || rollDayOrConvention == AQLString( "29" )
                      || rollDayOrConvention == AQLString( "14" )   || rollDayOrConvention == AQLString( "30" )
                      || rollDayOrConvention == AQLString( "15" )   || rollDayOrConvention == AQLString( "31" )
                      || rollDayOrConvention == AQLString( "16" )   )
            {
                tempRollDay                     = rollDayOrConvention.getIntValue();
                pRollDay                        = & tempRollDay;
            }
            else
            {
                throw AQLCoreInvalidData( "#Error: 'RollDayOrConvention' must be a day of the month or a date convention i.e. IMM or EOM (End of Month).", __FILE__, __LINE__ );
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
            throw AQLCoreInvalidData( "#Error: Unable to generate accrual dates with the parameters specified.", __FILE__, __LINE__ );
        }

        DateVector  paymentBaseDates;
        AQLString    payFrequency        = paymentFrequency;
        AQLString    payRollConvention   = paymentRollConvention;
        AQLString    payCalendar         = paymentCalendar;

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
            throw AQLCoreInvalidData( "#Error: Unable to generate the payment dates using the parameters specified.", __FILE__, __LINE__ );
        }

        // Schedule Data Placeholders
        DoubleMatrix    scheduleResults;
        DoubleVector    scheduleRow;

        AQLDate      thisStartDate           = AQLDate();
        AQLDate      thisEndDate             = AQLDate();
        AQLDate      thisFixingDateNoLag     = AQLDate();
        AQLDate      thisFixingDateWithLag   = AQLDate();
        AQLDate      thisPaymentDateNoLag    = AQLDate();
        AQLDate      thisPaymentDateWithLag  = AQLDate();
        double      thisAccrualPeriod       = 0.0;

        AQLDate      lastEndDate             = AQLDate();

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
                        throw AQLCoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
                    }

                    if ( paymentRowIndex == 0 || paymentRowIndex > paymentBaseDates.size() )
                    {
                        throw AQLCoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
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
                        throw AQLCoreInvalidData( "#Error: Unable to fix in Arrears, since floating paymennts cannot be made before their fixing dates.", __FILE__, __LINE__ );
                    }

                    if ( ( accrualRowIndex - 1 ) < 0 )
                    {
                        throw AQLCoreInvalidData( "#Error: Unable to generate fixing dates. Access Violation Error.", __FILE__, __LINE__ );
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
                        throw AQLCoreInvalidData( "#Error: Payments cannot be made before fixing dates. Please check if the schedule payment and fixing lags and other inputs are correct.", __FILE__, __LINE__ );
                    }

                    if ( thisStartDate > thisEndDate )
                    {
                        throw AQLCoreInvalidData( "#Error: Accrual start dates cannot be after the accrual end dates. Please check that the schedule inputs are correct.", __FILE__, __LINE__ );
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
                        throw AQLCoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
                    }

                    if ( paymentRowIndex == 0 || paymentRowIndex > paymentBaseDates.size() )
                    {
                        throw AQLCoreInvalidData( "#Error: Unable to build schedule. Incorrect accrual schedule dimensions.", __FILE__, __LINE__ );
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
                        throw AQLCoreInvalidData( "#Error: Payments cannot be made before fixing dates. Please check if the schedule payment and fixing lags and other inputs are correct.", __FILE__, __LINE__ );
                    }

                    if ( thisStartDate > thisEndDate )
                    {
                        throw AQLCoreInvalidData( "#Error: Accrual start dates cannot be after the accrual end dates. Please check that the schedule inputs are correct.", __FILE__, __LINE__ );
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
