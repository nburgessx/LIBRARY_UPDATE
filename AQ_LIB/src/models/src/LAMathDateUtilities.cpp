#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathDateUtilities.h"
#include "AQLFunctionUtilities.h"
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
#include "LAMathDateCalculations.h"
#include "AQLDataReference.h"
#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "AQLDataProcedure.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataMultiReference.h"
#include "ExceptionMacros.h"

#include <cmath>
#include <map>
#include <algorithm>

using namespace std;


//////////////////////////Date method//////////////////////////////////////////////////////////
//change excel date into AQLDate
AQLDate 
LAMathDateUtilities::getLADate( const int excel_date )
{
	AQLDate    ret_date( "19900101" );
	const int excel_base = 32874;
	ret_date.addDays( excel_date - excel_base );
	return ret_date;
}
//change excel date into AQLDate
AQLDate 
LAMathDateUtilities::getLADate(const AQLString& excel_date_str)
{
	AQLDate    ret_date( "19900101" );
	const int excel_base = 32874;
	ret_date.addDays( excel_date_str.getIntValue() - excel_base );
	return ret_date;
}
//change excel date into AQLString
AQLString 
LAMathDateUtilities::getLAStringDate( const int excel_date )
{
	AQLString  ret_str  = getLADate(excel_date).stringWithFormat("YYYYMMDD");
	return ret_str;
}

//change MDate into excel date
int 
LAMathDateUtilities::getExcelDate( const AQLDate & date )
{
	AQLDate    base( "19900101" );
	const int excel_base = 32874;
	return excel_base + base.intervalDays( date );
}

// Get Front Stub Date
// Note: To be able to calculate to roll dates correctly, we must provide the unadjusted start and end dates
StubDateAndType LAMathDateUtilities::getStubDateAndType( const AQLDate & unadjustedStartDate, const AQLDate & unadjustedEndDate, AQLString & term, const AQLPriceDataSlidingRule & busDayAdj, const AQLPriceDataCalendar & calendar, const AQLString* rollConvention, const etrading::StubTypeEnum & stubType )
{
	// Validation
	upper( term );
	AQ_REQUIRE( unadjustedStartDate <= unadjustedEndDate, "Invalid Cashflow Date: The Start Date '" + unadjustedStartDate.stringWithFormat("DD-MMM-YY") + "' must be before the End Date '" + unadjustedEndDate.stringWithFormat("DD-MMM-YY") + "'" )


	// 1.	Initialize Result - Default to Short Start if Missing
	//		*****************************************************
	StubDateAndType stubInfo;

	// Default to EndDate for below Boundary Conditions
	stubInfo.stubDate_			= unadjustedEndDate;
			
	// Use Short Start by Default if the stub is missing
	// Note: Below we apply adjust the Default StubType Market Convention, which may become LONG_START if stub days < 7 days
	stubInfo.stubTypeEnum_		= stubType == etrading::NONE_STUBTYPE ? etrading::SHORT_START_STUBTYPE : stubType;
			
	// Record if we are using the default stub type
	stubInfo.usingDefaultStub_	= stubType == etrading::NONE_STUBTYPE;

	// 2.	Populate StubDateAndType Parameters
	//		****************************************************************

	// isFrontStub
	stubInfo.isFrontStub_ = false;
	if( stubInfo.stubTypeEnum_ == etrading::SHORT_START_STUBTYPE || stubInfo.stubTypeEnum_ == etrading::LONG_START_STUBTYPE )
	{
		stubInfo.isFrontStub_ = true;
	}

	// isStartRoll
	stubInfo.isStartRoll_ = !stubInfo.isFrontStub_;

	// isShortStub
	stubInfo.isShortStub_ = false;
	if( stubInfo.stubTypeEnum_ == etrading::SHORT_START_STUBTYPE || stubInfo.stubTypeEnum_ == etrading::SHORT_END_STUBTYPE )
	{
		stubInfo.isShortStub_ = true;
	}

	// isHolidayAdjusted
	stubInfo.isHolidayAdjusted_ = true;
	if ( busDayAdj.getSlidingRule() == SlidingRuleType::SLIDING_RULE_NO_CHANGE )
	{
		stubInfo.isHolidayAdjusted_ = false;
	}

	// 3.	Boundary Condition(s) - Single Date
	//		***********************************
	
	// Single Date
	if ( unadjustedStartDate == unadjustedEndDate )
	{
		stubInfo.unadjustedSchedule_.push_back(unadjustedStartDate);
		stubInfo.unadjustedSchedule_.push_back(unadjustedEndDate);
		stubInfo.isRegularSchedule_ = true;
		return stubInfo;
	}

	// 4.	Front Stub - Roll Backwards from End Date
	//		*****************************************
	
	// Use 100 years of daily points as max search count guard
	int maxCount    = 40000; 
	int counter     = 0;

	if ( stubInfo.isFrontStub_ )
	{ 
		AQLDate previousStubDate = unadjustedEndDate;
		AQLDate thisStubDate		= unadjustedEndDate;
		AQLDate nextStubDate		= unadjustedEndDate;

		// Initialize Coupon Schedule
		stubInfo.unadjustedSchedule_.push_back(unadjustedEndDate);

		// Rolling Backwards from the EndDate
		while ( thisStubDate > unadjustedStartDate )
		{
			// Exit Condition
			nextStubDate = LAMathDateCalculations::getDate( thisStubDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), false, rollConvention ); // rollForwards = false
			if( nextStubDate <= unadjustedStartDate )
			{
				break;
			}

			// Update Stub Dates
			previousStubDate = thisStubDate;
			thisStubDate = nextStubDate;

			// Update Schedule - Don't double-count the end date
			if ( previousStubDate != unadjustedEndDate) stubInfo.unadjustedSchedule_.push_back(previousStubDate);

			// Counter Guard
			counter++;
			AQ_REQUIRE( counter < maxCount, "Unable to find the stub date for the stub type provided." )
		}

		// Adjust For Holidays and Update Stub Date
		// Note: Don't apply roll convention twice, since boundary start and end dates must not have roll convention applied to them
		// -------------------
		if ( stubInfo.isShortStub_ )
		{ 
			// Unadjusted Stub for Schedule
			if ( thisStubDate > unadjustedStartDate ) stubInfo.unadjustedSchedule_.push_back(thisStubDate);

			// *** Short Stubs ***
			thisStubDate		= LAMathDateCalculations::getDate( thisStubDate, "0D", busDayAdj, &calendar, false, NULL ); // rollForwards = false
			stubInfo.stubDate_ = thisStubDate;
			
		}
		else
		{
			// *** Long Stubs ***
			previousStubDate	= LAMathDateCalculations::getDate( previousStubDate, "0D", busDayAdj, &calendar, false, NULL ); // rollForwards = false
			stubInfo.stubDate_ = previousStubDate;
		}
		
		// Finalize Coupon Schedule - StartDate
		// Note: We need to reverse schedule order for start stubs as we are rolling backwards since isStartRoll = false
		stubInfo.unadjustedSchedule_.push_back(unadjustedStartDate);
		std::reverse( stubInfo.unadjustedSchedule_.begin(), stubInfo.unadjustedSchedule_.end() );

		// **************************************************************************************************
		// *** Default StubType Market Convention - Only Applies to Front Stubs ***
		// Default Stub is Short Start, however if stub is strictly less than 7 days the default should be Long Start
		// **************************************************************************************************
		if( stubInfo.usingDefaultStub_ )
		{
			const int stubDays = std::abs( unadjustedStartDate.intervalDays( thisStubDate ) );
			if( stubDays < 7 )
			{
				// Long Stub if Stub Days < 7 Days
				stubInfo.stubDate_		= previousStubDate;
				stubInfo.stubTypeEnum_	= etrading::LONG_START_STUBTYPE;
				stubInfo.isShortStub_	= false;
			}
		}

		// Check if Regular Schedule
		// Note: Don't apply roll convention twice, since boundary start and end dates must not have roll convention applied to them
		AQLDate adjustedStartDate = LAMathDateCalculations::getDate( unadjustedStartDate, "0D", busDayAdj, &calendar, false, NULL ); // rollForwards = false
		stubInfo.isRegularSchedule_ = false;
		if ( nextStubDate == adjustedStartDate )
		{
			stubInfo.isRegularSchedule_ = true;
		}
	}

	// 5.	Back Stub - Roll Forwards from Start Date
	//		******************************************
	else
	{
		AQLDate previousStubDate = unadjustedStartDate;
		AQLDate thisStubDate		= unadjustedStartDate;
		AQLDate nextStubDate		= unadjustedStartDate;

		// Initialize Coupon Schedule
		stubInfo.unadjustedSchedule_.push_back(unadjustedStartDate);

		// Rolling Forwards from the StartDate
		while ( thisStubDate < unadjustedEndDate )
		{
			// Exit Condition
			nextStubDate = LAMathDateCalculations::getDate( thisStubDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), true, rollConvention ); // IsAfter = true i.e. roll forwards;
			if( nextStubDate >= unadjustedEndDate )
			{
				break;
			}

			// Update Stub Dates
			previousStubDate = thisStubDate;
			thisStubDate = nextStubDate;

			// Update Schedule - Don't double-count the start date
			if ( previousStubDate != unadjustedStartDate) stubInfo.unadjustedSchedule_.push_back(previousStubDate);

			// Counter Guard
			counter++;
			AQ_REQUIRE( counter < maxCount, "Unable to find the stub date for the stub type provided." )
		}

		// Adjust For Holidays and Update Stub Date
		// Note: Don't apply roll convention twice, since boundary start and end dates must not have roll convention applied to them
		// -------------------
		if ( stubInfo.isShortStub_ )
		{ 
			// Unadjusted Stub for Schedule
			if ( thisStubDate < unadjustedEndDate ) stubInfo.unadjustedSchedule_.push_back(thisStubDate);

			// *** Short Stubs ***
			thisStubDate		= LAMathDateCalculations::getDate( thisStubDate, "0D", busDayAdj, &calendar, true, NULL ); // rollForwards = true
			stubInfo.stubDate_ = thisStubDate;
			
		}
		else
		{
			// *** Long Stubs ***
			previousStubDate	= LAMathDateCalculations::getDate( previousStubDate, "0D", busDayAdj, &calendar, true, NULL ); // rollForwards = true
			stubInfo.stubDate_ = previousStubDate;
		}

		// Finalize Coupon Schedule - EndStubDate & EndDate
		stubInfo.unadjustedSchedule_.push_back(unadjustedEndDate);

		// *** Default StubType Market Convention  - Only Applies to Front Stubs ***
		// No need to check for Default Stub Type

		// Check if Regular Schedule
		// Note: Don't apply roll convention twice, since boundary start and end dates must not have roll convention applied to them
		AQLDate adjustedEndDate = LAMathDateCalculations::getDate( unadjustedEndDate, "0D", busDayAdj, &calendar, false, NULL ); // rollForwards = false
		stubInfo.isRegularSchedule_ = false;
		if ( nextStubDate == adjustedEndDate )
		{
			stubInfo.isRegularSchedule_ = true;
		}
	}

	return stubInfo;

}

AQLDate LAMathDateUtilities::firstStubDateFromStubType( const AQLDate & startDate, const AQLDate & endDate, AQLString & term )
{
    upper( term );
    bool isAfter    = false; // This tells getDate to calculate dates backwards from the End Date
    
    AQLDate result = LAMathDateCalculations::getDate( endDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL );
    
    // Ensure the first stub is not before the start date
    if ( result < startDate )
        return startDate;

    int maxCount    = 20000;
    int counter     = 0;
    
    // Update the Stub Dates
    AQLDate longStubDate = result;
    AQLDate shortStubDate = result; 

    // We are rolling backwards in time
    while ( shortStubDate > startDate )
    {
        // Update Long Stub Date to the previous Short Stub Date
        longStubDate = shortStubDate;

        // Update the Short Stub Date
        shortStubDate = LAMathDateCalculations::getDate( shortStubDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL );

        // Update the result if the first stub is not before the start date else break out of the while loop
        if( shortStubDate > startDate )
        {
            result = longStubDate;
        }
        else 
        {
            // Case Where Long and Short Start Stubs give the same result i.e. we have a regular stub which is neither short or long
            if ( shortStubDate == startDate )
            {
                result = startDate;
            }
            break;
        }
        
        counter++;

        if ( counter >= maxCount )
            throw AQLCoreInvalidData("#Error: Unable to find the stub date for the stub type provided.",__FILE__,__LINE__);
    }

    return result;
}

AQLDate
LAMathDateUtilities::lastStubDateFromStubType( const AQLDate & startDate, const AQLDate & endDate, AQLString & term)
{
    upper( term );
    bool isAfter                    = true; // This tells getDate to calculate dates forwards from the Start Date

    AQLDate result = LAMathDateCalculations::getDate( startDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL );

    // Ensure the last stub is not after the end date
    if ( result > endDate )
        return endDate;

    int maxCount  = 20000;
    int counter = 0;
    
    // Update the Stub Dates
    AQLDate longStubDate = result;
    AQLDate shortStubDate = result; 

    // We are rolling forwards in time
    while ( shortStubDate < endDate )
    {
        // Update Long Stub Date to the previous Short Stub Date
        longStubDate = shortStubDate;

        // Update the Short Stub Date
        shortStubDate = LAMathDateCalculations::getDate( shortStubDate, term, AQLPriceDataSlidingRule(), &AQLPriceDataCalendar(), isAfter, NULL );
        
        // Update the result if the last stub is not after the end date else break out of the while loop
        if( shortStubDate < endDate ) 
        {
            result = longStubDate;
        }
        else
        {
            // Case Where Long and Short Start Stubs give the same result i.e. we have a regular stub which is neither short or long
            if ( shortStubDate == endDate )
            {
                result = endDate;
            }
            break;
        }
        
        counter++;

        if ( counter >= maxCount )
            throw AQLCoreInvalidData("#Error: Unable to find the stub date for the stub type provided.",__FILE__,__LINE__);
    }

    return result;
}

DateVector
LAMathDateUtilities::calcDatesWithLag( const DateVector &           dates,
                                       const AQLString &             term,
                                       const AQLPriceDataSlidingRule &    slidingRule,
                                       const AQLPriceDataCalendar *       pCalendar,
                                       const bool &                 isAfter,
                                       const AQLString *             rollConvention )
{
    DateVector results;

    for ( unsigned int i = 0; i < dates.size(); ++i )
    {
        results.push_back( LAMathDateCalculations::getDate( dates[i], term, slidingRule, pCalendar, isAfter, rollConvention ) );
    }

    return results;
}

// Generate a Date Schedule with appropriate use of stubs
// Note that there is a duplicate method LADateScheduleHelpers::generateSchedule
// Default Short/Long Start is determined by LAMathDateUtilities::getStubDateAndType
DateVector LAMathDateUtilities::generateSchedule(const AQLDate& unadjustedStart,
												 const AQLDate& unadjustedEnd,
												 AQLString& data_frequency,
												 AQLString& slidingRuleString,
												 AQLString& calendarString,
												 const AQLDate* firstStubDate,
												 const AQLDate* lastStubDate,
												 const int* rollDay,
												 const bool is_start_roll,
												 const AQLString* rollConvention,
												 const AQLString* stubType)
{
    //change nospace & upper
    upper(data_frequency);
    upper(slidingRuleString);
    upper(calendarString);

    AQLPriceDataSlidingRule slidingRule;
    slidingRule.convertFromString(slidingRuleString);
        
    AQLPriceDataCalendar calendar;
    calendar.convertFromString(calendarString);
        
	DateVector results;

    // ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 
    // OPTIONAL: Apply Stub Type logic
    //
    // ////////////////////////////////////////////////////////////////////////////////////////////////////

    // Added to resolve const keyword issues
    bool isStartRoll = is_start_roll;

    AQLDate thisFirstStubDate;
    AQLDate thisLastStubDate;

    AQLDate* pFirstStubDate = NULL;
    AQLDate* pLastStubDate = NULL;

    if (firstStubDate != NULL)
    {
        thisFirstStubDate = *firstStubDate;
        pFirstStubDate = &thisFirstStubDate;
    }

    if (lastStubDate != NULL)
    {
        thisLastStubDate = *lastStubDate;
        pLastStubDate = &thisLastStubDate;
    }

    // Get the equivalent term from data_frequency
    AQLString term;
    if (data_frequency == "ANNUAL")
        term = "1Y";
    else if (data_frequency == "SEMI-ANNUAL")
        term = "6M";
    else if (data_frequency == "QUARTERLY")
        term = "3M";
    else if (data_frequency == "MONTHLY")
        term = "1M";
    else if (data_frequency == "WEEKLY")
        term = "1W";
	else if (data_frequency == "BUSINESS_DAYS" || data_frequency == "DAILY")
        term = "1D";
    else if ((data_frequency == "NONE" || data_frequency == "LUNAR") && stubType == NULL);
    else
        throw AQLCoreInvalidData("#Error: Frequency, must be Annual, Semi-Annual, Quarterly, Monthly, Weekly or Daily.", __FILE__, __LINE__);

	// Initialize StubDateAndType Struct
	StubDateAndType stubInfo;

	// Default Stub Type
	if (stubType == NULL && firstStubDate == NULL && lastStubDate == NULL)
	{
		// Market Default to Short Start Stub, however if Stub Days is < 7 Days the Default becomes Long Start Stub
		stubInfo = LAMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::NONE_STUBTYPE );

		isStartRoll		= stubInfo.isStartRoll_;
		pFirstStubDate	= &stubInfo.stubDate_;
		pLastStubDate	= NULL;
	}

    // Override the stubs dates to generate the correct stub types
    if (stubType != NULL)
    {
		AQLString stubTypeString = *stubType;
        upper(stubTypeString);

        if (stubTypeString == "NONE")
        {
			if ( firstStubDate == NULL || lastStubDate == NULL )
			{ 
				// Market Default to Short Start Stub, however if Stub Days is < 7 Days the Default becomes Long Start Stub
				stubInfo = LAMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::NONE_STUBTYPE );
				
				isStartRoll		= stubInfo.isStartRoll_;
				pFirstStubDate	= &stubInfo.stubDate_;
				pLastStubDate	= NULL;
			}
        }
        else if (stubTypeString == "SHORTSTART" || stubTypeString == "SS")
        {
			AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )

            stubInfo = LAMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::SHORT_START_STUBTYPE );
                
			isStartRoll		= stubInfo.isStartRoll_;
            pFirstStubDate	= &stubInfo.stubDate_;
            pLastStubDate	= NULL;
        }
        else if (stubTypeString == "LONGSTART" || stubTypeString == "LS")
        {
			AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )

            stubInfo = LAMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::LONG_START_STUBTYPE );
                
			isStartRoll		= stubInfo.isStartRoll_;
            pFirstStubDate	= &stubInfo.stubDate_;
            pLastStubDate	= NULL;
        }
        else if (stubTypeString == "SHORTEND" || stubTypeString == "SE")
        {
			AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )

            stubInfo = LAMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::SHORT_END_STUBTYPE );

            isStartRoll		= stubInfo.isStartRoll_;
            pFirstStubDate	= NULL;
            pLastStubDate	= &stubInfo.stubDate_;
        }
        else if (stubTypeString == "LONGEND" || stubTypeString == "LE")
        {
			AQ_THROW_IF(firstStubDate != NULL || lastStubDate != NULL, "Must not specifiy 'StubType' with 'FirstStubDate' or 'LastStubDate'." )

            stubInfo = LAMathDateUtilities::getStubDateAndType(unadjustedStart, unadjustedEnd, term, slidingRule, calendar, rollConvention, etrading::LONG_END_STUBTYPE );

            isStartRoll		= stubInfo.isStartRoll_;
            pFirstStubDate	= NULL;
            pLastStubDate	= &stubInfo.stubDate_;
        }
        else
        {
            AQ_THROW("Stub Type must be None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).")
        }
    }
    // ////////////////////////////////////////////////////////////////////////////////////////////////////

    LAMathDateCalculations::generateSchedule(unadjustedStart,
											 unadjustedEnd,
											 data_frequency,
											 true,
											 pFirstStubDate,
											 pLastStubDate,
											 rollDay,
											 results,
											 &slidingRule,
											 &calendar,
											 isStartRoll,
											 rollConvention );

    if (results.front() != unadjustedStart)
	{
		results.insert(results.begin(), unadjustedStart);
	}

    return results;
}

/*!
    @brief Function to build a dates schedule for swap pricing

    @param[in] testDate
    @param[in] throwString
    
    @returns True if 'dateToValidate' is valid and false otherwise

*/
bool
LAMathDateUtilities::isValidDate( const AQLDate & dateToValidate )
{
    const unsigned short  mDay    = dateToValidate.dayOfMonth();
    const unsigned short  mMonth  = dateToValidate.monthOfYear();
    const unsigned short  mYear   = dateToValidate.yearOfEra();
    const unsigned short  mLeap   = dateToValidate.isLeapYear();

    // number of days each month and the number of days from the beginning of the year in an usual yer and a leap year(0 start)
    const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
        {{{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        { 0, 31, 59, 90,120,151,181,212,243,273,304,334}},
        {{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        { 0, 31, 60, 91,121,152,182,213,244,274,305,335}}};

    // Test for a valid date
    if ( mYear <= 1900 )
        return false;   //dateToValidate is invalid

    if ( mYear == 0) 
        return false;   //dateToValidate is invalid

    if ( mMonth == 0    ||  mMonth > 12 )
        return false;   //dateToValidate is invalid

    if( mDay == 0       ||  mDay > ( int )( LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[ mLeap ][ 0 ][ mMonth - 1 ] ) ) 
        return false;   //dateToValidate is invalid
    
    // dateToValidate is valid
    return true;
}

AQLDate
LAMathDateUtilities::getDateFromTerm(const AQLDate& fromdate, const double termy, const AQLPriceDataDayCount& daycount, bool includelast)
{
	AQLPriceDataConvention convention( daycount.getDayCount(), CONT ); // Continous compounding rate convention

	AQLDate ret = fromdate;
	double fullDay = LAPriceCFGenUtility::round( convention.getDayTerm( ret, termy ), RoundFunction::ROUND, 0 );
	ret.addDays( fullDay );
	
	return ret;
}

AQLDate 
LAMathDateUtilities::getDateFromTerm(AQLDate& fromdate, double termy, AQLString& dayCountString, bool includelast)
{
	includelast;
	//change nospace & upper
	upper(dayCountString);

	AQLPriceDataDayCount daycount;
	daycount.setDayCount( AQLCoreComponentManager::getDayCount( dayCountString ) );
	AQLDate ret = getDateFromTerm( fromdate, termy, daycount );
	return ret;
}

double 
LAMathDateUtilities::getDayFromTerm(AQLDate& fromdate, double termy, AQLString& daycount, bool includelast)
{
	//change nospace & upper
	upper(daycount);

	AQLPriceDataDayCount dc;
	dc.setDayCount(AQLCoreComponentManager::getDayCount(daycount));
	AQLPriceDataConvention conv(dc.getDayCount(),CONT);
	
	//false means not includelast
	double ret = dc.getDayTerm(fromdate,termy,includelast);
	return ret;
}
double 
LAMathDateUtilities::getTermFromDay(AQLDate& fromdate, double termd, AQLString& daycount, bool includelast)
{
	//change nospace & upper
	upper(daycount);

	AQLPriceDataDayCount dc;
	dc.setDayCount(AQLCoreComponentManager::getDayCount(daycount));
	AQLPriceDataConvention conv(dc.getDayCount(),CONT);
	
	//false means not includelast
	double ret = dc.getTerm(fromdate,termd,includelast);
	return ret;
}
double 
LAMathDateUtilities::getTerm(const AQLDate& fromdate, const AQLDate& todate, AQLString& daycount, bool includelast, 
							 const AQLString* frequency,
							 const AQLString* Calendar,
							 const AQLString* SlidingRule,
							 const std::vector<AQLDate>* startdates,
							 const std::vector<AQLDate>* enddates)
{
	//change nospace & upper
	upper(daycount);

	AQLPriceDataDayCount dc;
	dc.setDayCount(AQLCoreComponentManager::getDayCount(daycount));

	if(dc.getDayCount() == ACT_ACT_ICMA)
	{
		DateMatrix regular_startenddates = LAMathDateUtilities::calcRegularDates(*frequency, *Calendar, *SlidingRule, *startdates, *enddates);
		dc.setCouponsInYear(12/LAMathDateCalculations::getPeriodFrequencyInMonths(*frequency));
		dc.setCouponStartDates(regular_startenddates[0]);
		dc.setCouponEndDates(regular_startenddates[1]);
	}

	AQLPriceDataConvention conv(dc.getDayCount(),CONT);
	//false means not includelast
	double ret = dc.getTerm(fromdate,todate,includelast);
	return ret;
}

AQLDate
LAMathDateUtilities::getDate(const AQLDate& basedate, const AQLString& term, const AQLString& slidingrule, const AQLString& calendar)
{
	//calendar
	AQLPriceDataCalendar cal;
	cal.convertFromString(calendar);
	
    //sliding rule
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(slidingrule);
	
	AQLDate ret = LAMathDateCalculations::getDate(basedate,term,sr,&cal,true); 
	return ret;
}

AQLDate
LAMathDateUtilities::getDateWithRollConv(const AQLDate& basedate, const AQLString& term, const AQLString& slidingRule, const AQLString& calendar, const AQLString* roll_conv)
{
    //calendar
	AQLPriceDataCalendar cal;
	cal.convertFromString(calendar);
	
    //sliding rule
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(slidingRule);

    AQLDate ret = LAMathDateCalculations::getDate(basedate, term, sr, &cal, true, roll_conv);
	
    return ret;
}

DateVector
LAMathDateUtilities::getMultiDate(const DateVector& basedate, const AQLString& term, const AQLString& slidingrule, const AQLString& calendar, const AQLString* roll_conv)
{
	//calendar
	AQLPriceDataCalendar cal;
	cal.convertFromString(calendar);
	//sliding rule
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(slidingrule);
	
	DateVector ret;
	for(size_t i=0;i<basedate.size();i++)
	{
		ret.push_back(LAMathDateCalculations::getDate(basedate[i], term, sr, &cal, true, roll_conv));
	}
	return ret;
}

AQLDate
LAMathDateUtilities::getDateWithRoll(AQLDate& basedate, AQLString& term, AQLString& slidingrule, AQLString& calendar, int roll)
{
	//change nospace & upper
	upper(term);
	upper(slidingrule);
	upper(calendar);
	//sliding rule
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(slidingrule);
	//calendar
	AQLPriceDataCalendar cal;
	cal.convertFromString(calendar);

	int y,m,d,w;
	LAMathDateCalculations::termStrtoYMDW(term,y,m,d,w);
	y*=roll;
	m*=roll;
	d*=roll;
	AQLString multiterm = AQLString(y)+"y"+AQLString(m)+"m"+AQLString(d)+"d";

	AQLDate ret = LAMathDateCalculations::getDate(basedate,multiterm,sr,&cal,true);
	return ret;
}

AQLDate
LAMathDateUtilities::getIMMDate1(const int& year, const int& month, AQLString& calendar, AQLString& slidingRule)
{
	upper(slidingRule);
	upper(calendar);
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(slidingRule);
	AQLPriceDataCalendar cal;
	cal.convertFromString(calendar);

	AQLDate date = LAMathDateCalculations::getIMMDate(year, month);
	return sr.getDate(date, cal);
}

AQLDate
LAMathDateUtilities::getIMMDate2(const int& year, const int& number, AQLString& calendar, AQLString& slidingRule)
{
	if (number < 0 || number > 5) throw AQLCoreInvalidData("IMM Dates in a year are 4 days.", __FILE__, __LINE__);

	upper(slidingRule);
	upper(calendar);
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(slidingRule);
	AQLPriceDataCalendar cal;
	cal.convertFromString(calendar);

	AQLDate date = LAMathDateCalculations::getIMMDate(year, number * 3);
	return sr.getDate(date, cal);
}

AQLDate
LAMathDateUtilities::getIMMDate3(const AQLDate& basedate, const int& number, AQLString& calendar, AQLString& slidingRule)
{
	upper(slidingRule);
	upper(calendar);
	AQLPriceDataSlidingRule sr;
	sr.convertFromString(slidingRule);
	AQLPriceDataCalendar cal;
	cal.convertFromString(calendar);

	int y_baseDate = basedate.yearOfEra();
	int m_baseDate = basedate.monthOfYear();

	AQLDate nextIMMDate; 
	nextIMMDate.setYear(y_baseDate); 
	nextIMMDate.setMonth(m_baseDate);

	if (m_baseDate % 3 == 0)
	{
		AQLDate IMMDate_baseDateMonth = LAMathDateCalculations::getIMMDate(y_baseDate, m_baseDate);
		IMMDate_baseDateMonth = sr.getDate(IMMDate_baseDateMonth, cal);
		if (IMMDate_baseDateMonth <= basedate) nextIMMDate.addMonths(3);
	}
	else if (m_baseDate % 3 == 1) 
	{
		nextIMMDate.addMonths(2);
	}
	else if (m_baseDate % 3 == 2) 
	{
		nextIMMDate.addMonths(1);
	}
	nextIMMDate.addMonths((number-1)*3);	
	nextIMMDate = LAMathDateCalculations::getIMMDate(nextIMMDate.yearOfEra(), nextIMMDate.monthOfYear());
	return sr.getDate(nextIMMDate, cal);
}

// calc regular (non-stub) payment dates for daycount ACT/ACT.ICMA
DateMatrix
LAMathDateUtilities::calcRegularDates(const AQLString& frequency,
									  const AQLString& Calendar,
									  const AQLString& SlidingRule,
								      const std::vector<AQLDate>& startdates,
								      const std::vector<AQLDate>& enddates)
{
	AQLPriceDataCalendar calendar;
	calendar.convertFromString(Calendar);
	AQLPriceDataSlidingRule slidingrule;
	slidingrule.convertFromString(SlidingRule);
	AQLString term = (AQLString)(LAMathDateCalculations::getPeriodFrequencyInMonths(frequency)) + "M";

	bool is_sorted_Start = true;
	bool is_sorted_End = true;
	for(unsigned int i = 1; i < startdates.size(); ++i)
	{
		if(startdates[i-1] >= startdates[i])
		{
			is_sorted_Start = false;
		}
	}
	for(unsigned int i = 1; i < enddates.size(); ++i)
	{
		if(enddates[i-1] >= enddates[i])
		{
			is_sorted_End = false;
		}
	}

	if(startdates.size() != enddates.size() || 
	   startdates.size() == 0 ||
	   !is_sorted_Start ||
	   !is_sorted_End)
	{
		AQLString msg = "Illegal StartDates and/or EndDates.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// merge StartDates and EndDates as a single date vector
	std::vector<AQLDate> startenddates;
	std::set_union(startdates.begin(), startdates.end(), 
					enddates.begin(), enddates.end(),
					std::inserter(startenddates, startenddates.end()));

	if(startenddates.size() != startdates.size() + 1) // case when Start[i+1] does not coincide with End[i]
	{
		AQLString msg = "Illegal StartDates and/or EndDates.";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	std::vector<AQLDate> regular_startdates;
	std::vector<AQLDate> regular_enddates;
	for(unsigned int i = 0; i < startdates.size(); ++i)
	{
		AQLDate startdate = startdates[i];
		AQLDate enddate = enddates[i];

		if(i == 0)
		{
			regular_startdates.push_back(LAMathDateCalculations::getDate(enddate, "-" + term, slidingrule, &calendar, true));
			regular_enddates.push_back(enddate);
			while(regular_startdates.back() > startdate) // Long at start
			{
				regular_enddates.push_back(regular_startdates.back());
				regular_startdates.push_back(LAMathDateCalculations::getDate(regular_enddates.back(), "-" + term, slidingrule, &calendar, true));
			}
		}
		else if(i == startdates.size() - 1)
		{
			regular_startdates.push_back(startdate);
			regular_enddates.push_back(LAMathDateCalculations::getDate(startdate, term, slidingrule, &calendar, true));
			while(regular_enddates.back() < enddate) // Long at end
			{
				regular_startdates.push_back(regular_enddates.back());
				regular_enddates.push_back(LAMathDateCalculations::getDate(regular_startdates.back(), term, slidingrule, &calendar, true));
			}
		}
		else
		{
			regular_startdates.push_back(startdate);
			regular_enddates.push_back(enddate);
		}
	}
	std::sort(regular_startdates.begin(), regular_startdates.end());
	std::sort(regular_enddates.begin(), regular_enddates.end());

	DateMatrix ret;
	ret.push_back(regular_startdates);
	ret.push_back(regular_enddates);
	return ret;
}

bool is_last_business_day_temp(const AQLDate& d, const AQLString& cal)
{
	const AQLDate next_day = LAMathDateUtilities::getDate(d, "1d", "FOLLOWING", cal);
	return next_day.monthOfYear() != d.monthOfYear();
}

int StringToMonthInteger(AQLString mstr)
{
    int m;
    if (mstr == "Jan") m = 1;
    else if (mstr == "Feb") m = 2;
    else if (mstr == "Mar") m = 3;
    else if (mstr == "Apr") m = 4;
    else if (mstr == "May") m = 5;
    else if (mstr == "Jun") m = 6;
    else if (mstr == "Jul") m = 7;
    else if (mstr == "Aug") m = 8;
    else if (mstr == "Sep") m = 9;
    else if (mstr == "Oct") m = 10;
    else if (mstr == "Nov") m = 11;
    else if (mstr == "Dec") m = 12;
    else throw AQLCoreAppError("invalid month", __FILE__, __LINE__);

    return m;
}

AQLString FrequencyToTerm(AQLString frequency)
{
	AQLString frequency_(frequency);
	frequency_.toUpper();
	if (frequency_ == BUSINESS_DAYS || frequency_ == DAILY)
		return "1D";
	else if (frequency_ == WEEKLY)
		return "1W";
	else if (frequency_ == LUNAR)
		return "28D";
	else if (frequency_ == MONTHLY)
		return "1M";
	else if (frequency_ == QUARTERLY)
		return "3M";
	else if (frequency_ == SEMI_ANNUAL)
		return "6M";
	else if (frequency_ == ANNUAL)
		return "12M";
	else
		throw AQLCoreInvalidData("Unknown frequency in conversion to Term", __FILE__, __LINE__);
}

AQLPriceDataDayCount ModelDaycount()
{
	AQLString daycountConvention = AQLString("ACT/365_ISDA");
	return Daycount(daycountConvention);
}

double ModelTime(AQLDate valDate, AQLDate targetDate)
{
	AQLPriceDataDayCount dayCount = ModelDaycount();
	return YearFraction(dayCount, valDate, targetDate);
}

AQLDate LAStringToDate(AQLString date)
{
	AQLDate ret;
	int slushCheck = date.findString("/");
	if (slushCheck == -1)
	{
		ret = LAMathDateUtilities::getLADate(date);
	}
	else
	{
		if (slushCheck == 4) date.remove(4, 1);
		else throw AQLCoreInvalidData("Input error", __FILE__, __LINE__);

		slushCheck = date.findString("/");
		if (slushCheck == 6) { date.remove(6, 1); }
		else if (slushCheck == 5)
		{
			date.remove(5, 1);
			date.insert(4, "0");
		}
		else throw AQLCoreInvalidData("Input error", __FILE__, __LINE__);

		if (date.size() == 7)
		{
			date.insert(6, "0");
		}
		else if (date.size() != 8) throw AQLCoreInvalidData("Input error", __FILE__, __LINE__);
		AQLDate ret_(date.getCString()); ret = ret_;
	}

	return ret;
}

double YearFraction(AQLPriceDataDayCount daycount, AQLDate valDate, AQLDate date)
{
	return daycount.getTerm(valDate, date, false);
}

AQLPriceDataDayCount Daycount(AQLString daycountConvention)
{
	AQLFunctionUtilities::upperdelspace(daycountConvention);
	AQLPriceDataDayCount dc;
	dc.setDayCount(AQLCoreComponentManager::getDayCount(daycountConvention));
	return dc;
}

double TermToYearLength(AQLString term)
{
    size_t termSize = term.size();
    AQLString unit = term.subString(termSize - 1, termSize - 1);
    double length = term.subString(0, termSize - 2).getDoubleValue();
    if (unit == "Y")
        return length;
    else if (unit == "M")
        return length / 12.0;
    else
        throw AQLCoreInvalidData("Invalid term in conversion to year length", __FILE__, __LINE__);
}

int TermToMonthLength(AQLString term)
{
    size_t termSize = term.size();
    AQLString unit = term.subString(termSize - 1, termSize - 1);
    int length = term.subString(0, termSize - 2).getIntValue();
    if (unit == "M")
        return length;
    else if (unit == "Y")
        return 12 * length;
    else
        throw AQLCoreInvalidData("Invalid term in conversion to month length", __FILE__, __LINE__);
}


AQLDate CalendarAdvance(AQLDate baseDate, AQLString term, AQLPriceDataSlidingRule slidingRule, AQLPriceDataCalendar calendar)
{
    AQLString* roll_conv = 0;
    return LAMathDateCalculations::getDate(baseDate, term, slidingRule, &calendar, true, roll_conv);
}
