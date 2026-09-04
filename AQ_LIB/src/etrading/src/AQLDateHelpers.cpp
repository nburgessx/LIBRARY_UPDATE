//
// AQLDateHelpers.cpp
//

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

// This Include
#include "AQLDateHelpers.h"              // was called #include "AQLMathDateCalcUti1ity.h"
#include "AQLDateScheduleHelpers.h"      // was called #include "AQLMathDateFuncUti1ity.h"

// Internal Includes
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathFXUtility.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "ExceptionMacros.h"

// External Includes
#include <algorithm>
#include <cmath>
#include <cassert>

using namespace std;

// TODO: Remove these defines statements, all of them! It's bad practice!
#define	BUSINESS_DAYS	"BUSINESS_DAYS"
#define	DAILY			"DAILY"
#define	WEEKLY			"WEEKLY"
#define	NONE			"NONE"
#define ANNUAL		    "ANNUAL"
#define SEMI_ANNUAL	    "SEMI-ANNUAL"
#define QUARTERLY	    "QUARTERLY"
#define MONTHLY		    "MONTHLY"
#define LUNAR		    "LUNAR"

#define ON "ON"
#define TN "TN"
#define SN "SN"

#define F_FUTURE_MONTH "F"
#define G_FUTURE_MONTH "G"
#define H_FUTURE_MONTH "H"
#define J_FUTURE_MONTH "J"
#define K_FUTURE_MONTH "K"
#define M_FUTURE_MONTH "M"
#define N_FUTURE_MONTH "N"
#define Q_FUTURE_MONTH "Q"
#define U_FUTURE_MONTH "U"
#define V_FUTURE_MONTH "V"
#define X_FUTURE_MONTH "X"
#define Z_FUTURE_MONTH "Z"

#define ROLLCONV_IMM        "IMM"
#define ROLLCONV_NORMAL     "NORMAL"
#define ROLLCONV_STARTDATE  "STARTDATE"
#define ROLLCONV_ENDDATE    "ENDDATE"
#define ROLLCONV_EOM        "EOM"
#define ROLLCONV_LUNAR      "LUNAR"


namespace
{
	const RollConventionEnum toRollConventionEnum( const AQLString& enumString )
	{
		AQLString uCaseString("");
		if (enumString != NULL) uCaseString = enumString;
		uCaseString.toUpper();

		if ( uCaseString == "" )
		{
			return ROLLCONV_ENUM_NONE;
		}
		if ( uCaseString == ROLLCONV_IMM )
		{
			return ROLLCONV_ENUM_IMM;
		}
		if ( uCaseString == ROLLCONV_NORMAL )
		{
			return ROLLCONV_ENUM_NORMAL;
		}	
		if ( uCaseString == ROLLCONV_STARTDATE )
		{
			return ROLLCONV_ENUM_STARTDATE;
		}
		if ( uCaseString == ROLLCONV_ENDDATE )
		{
			return ROLLCONV_ENUM_ENDDATE;
		}
		if ( uCaseString == ROLLCONV_EOM )
		{
			return ROLLCONV_ENUM_EOM;
		}
		if ( uCaseString == ROLLCONV_LUNAR )
		{
			return ROLLCONV_ENUM_LUNAR;
		}
		else
		{
			AQLString msg;
			msg += "#Error: Unknown roll convention; ";
			msg += enumString;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}
}


namespace etrading
{

    const AQLString toString( const RollConventionEnum enumValue )
    {
        switch( enumValue )
        {
            case ROLLCONV_ENUM_NONE:
                return "";
                break;

            case ROLLCONV_ENUM_IMM:
                return ROLLCONV_IMM;
                break;

            case ROLLCONV_ENUM_NORMAL:
                return ROLLCONV_NORMAL;
                break;

            case ROLLCONV_ENUM_STARTDATE:
                return ROLLCONV_STARTDATE;
                break;

		    case ROLLCONV_ENUM_ENDDATE:
                return ROLLCONV_ENDDATE;
                break;

		    case ROLLCONV_ENUM_EOM:
                return ROLLCONV_EOM;
                break;

		    case ROLLCONV_ENUM_LUNAR:
                return ROLLCONV_LUNAR;
                break;

            default:
		    {
			    AQLString msg = "#Error: RollConventionEnum number '";
			    msg += enumValue;
			    msg += "' is not convertable to a string.";
			    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);;
			    break;
		    }
        }
    }

    // each leap year and not, day of month and day from head of this year to head of this month (from 0)
    // (copied by AQLDate.cpp) 
    static const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
        {{{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
          { 0, 31, 59, 90,120,151,181,212,243,273,304,334}},
         {{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
          { 0, 31, 60, 91,121,152,182,213,244,274,305,335}}};


    // Get date adjusted for holidays
    AQLDate AQLDateHelpers::getAdjDate(const AQLPriceDataSlidingRule* busDayAdj, const AQLPriceDataCalendar* calendar, const AQLDate &date)
    {
	    if (busDayAdj != NULL && calendar != NULL)
	    {
		    return busDayAdj->getDate(date, *calendar);
	    }
	    else
	    {
		    return date;
	    }
    }


    void
    AQLDateHelpers::setDate(const int* rollDay, AQLDate &date, const RollConventionEnum rollConventionEnum)
    {
	    if ( rollConventionEnum == ROLLCONV_ENUM_NONE ||
		     rollConventionEnum == ROLLCONV_ENUM_NORMAL ||
		     rollConventionEnum == ROLLCONV_ENUM_STARTDATE ||
		     rollConventionEnum == ROLLCONV_ENUM_ENDDATE ||
		     rollConventionEnum == ROLLCONV_ENUM_LUNAR )
	    {
		    if(rollDay == NULL) return;
		    date.setDay(min(*rollDay, (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date.yearOfEra())][0][date.monthOfYear() - 1]));
	    }
	    else if ( rollConventionEnum == ROLLCONV_ENUM_EOM )
	    {
		    date.setDay((int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date.yearOfEra())][0][date.monthOfYear() - 1]);
	    }
	    else if (rollConventionEnum == ROLLCONV_ENUM_IMM)
	    {
		    date = getIMMDate(date.yearOfEra(), date.monthOfYear(), (date.monthOfYear() % 3) != 0);
	    }
	    else
		{
		    AQ_THROW("Invalid Roll Convention: Must be NORMAL, IMM or EOM")
	    }
    }

    /*!
	    @brief calculate dates based basic pattern
		
		Firstly we build a schedule of regular coupons, Secondly we add the start date if we have a front stub ...
		... or the end date if we have a back stub

        @param[in] start				start date
	    @param[in] end					end date
	    @param[in] data_frequency		payment frequency
	    @param[in] inArrears			arrear or not(bool)
	    @param[in] rollDay				pointer of payment day
	    @param[in] busDayAdj			pointer of SlidingRule
	    @param[in] calendar				pointer of Calender

        @return DateVector of grid points
    */
    DateVector AQLDateHelpers::generateRegularSchedule( const AQLDate& start, 
													   const AQLDate& end,
													   const AQLString& data_frequency,
													   const bool inArrears,
													   const int* rollDay,
													   const AQLPriceDataSlidingRule* busDayAdj,
													   const AQLPriceDataCalendar* calendar,
													   const bool isStartRoll,
													   const AQLString* roll_convention )
    {
		std::deque<AQLDate> resultDates;

	    AQLDate unadjustedDate;
	    AQLDate adjustedDate;

	    RollConventionEnum rollConventionEnum = roll_convention == NULL ? ROLLCONV_ENUM_NONE : toRollConventionEnum( *roll_convention );

	    if (isStartRoll)
	    {
		    unadjustedDate = start;
		    if (data_frequency == BUSINESS_DAYS || data_frequency == DAILY )
		    {
			    unadjustedDate.addDays(1);

			    for ( ;unadjustedDate < end ;unadjustedDate.addDays(1))
			    {
				    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
				    if (unadjustedDate == adjustedDate)
					    resultDates.push_back(adjustedDate);
			    }
		    }
		    else if (data_frequency == WEEKLY)
		    {
			    unadjustedDate.addDays(7);

			    for ( ;unadjustedDate < end ;unadjustedDate.addDays(7))
			    {
				    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
				    resultDates.push_back(adjustedDate);
			    }
		    }
		    else if (data_frequency == LUNAR)
		    {
			    unadjustedDate.addDays(28);

			    for ( ;unadjustedDate < end ;unadjustedDate.addDays(28))
			    {
				    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
				    resultDates.push_back(adjustedDate);
			    }
		    }
		    else 
		    {
			    if (rollConventionEnum ==ROLLCONV_ENUM_LUNAR)
			    {
				    int days = 0;
				    if (data_frequency == MONTHLY)
				    {
					    days = 28;
				    }
				    else if (data_frequency == QUARTERLY)
				    {
					    days = 91;
				    }
				    else if (data_frequency == SEMI_ANNUAL)
				    {
					    days = 182;
				    }
				    else if (data_frequency == ANNUAL)
				    {
					    days = 364; //=13*28
				    }
				    else
				    {
					    AQ_THROW("Invalid Frequency: Must be Annual, Semi-Annual, Quarterly or Monthly, however '" + data_frequency + "' provided");		
				    }
				    unadjustedDate.addDays(days);
				    for (unsigned int i = 1; unadjustedDate < end; i++)
				    {
					    unadjustedDate = start; 
					    unadjustedDate.addDays(i * days);
						if ( unadjustedDate >= end ) break;

					    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
					    resultDates.push_back(adjustedDate);
				    }
			    }
			    else
			    {
				    int months = 0;
				    if (data_frequency == MONTHLY)
				    {
					    months = 1;
				    }
				    else if (data_frequency == QUARTERLY)
				    {
					    months = 3;
				    }
				    else if (data_frequency == SEMI_ANNUAL)
				    {
					    months = 6;
				    }
				    else if (data_frequency == ANNUAL)
				    {
					    months = 12;
				    }
				    else
				    {
						AQ_THROW("Invalid Frequency: Must be Annual, Semi-Annual, Quarterly, however '" + data_frequency + "' provided")
				    }
				    unadjustedDate.addMonths(months);
				    for (unsigned int i = 1; unadjustedDate < end; i++)
				    {
					    unadjustedDate = start; 
					    unadjustedDate.addMonths(i * months);
						setDate( rollDay, unadjustedDate, rollConventionEnum );
						if ( unadjustedDate >= end ) break;

						adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
					    resultDates.push_back(adjustedDate);
				    }
			    }
		    }
	    }
	    else
	    {
		    AQLDate unadjustedDate = end;
		    if (data_frequency == BUSINESS_DAYS || data_frequency == DAILY)
		    {
			    unadjustedDate.addDays(-1);

			    for ( ;unadjustedDate > start ;unadjustedDate.addDays(-1))
			    {
				    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
				    if (unadjustedDate == adjustedDate)
					    resultDates.push_front(adjustedDate);
			    }
		    }
		    else if (data_frequency == WEEKLY)
		    {
			    unadjustedDate.addDays(-7);

			    for ( ;unadjustedDate > start ;unadjustedDate.addDays(-7))
			    {
				    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate); 
				    resultDates.push_front(adjustedDate);
			    }
		    }
		    else if (data_frequency == LUNAR)
		    {
			    unadjustedDate.addDays(-28);

			    for ( ;unadjustedDate > start ;unadjustedDate.addDays(-28))
			    {
				    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate); 
				    resultDates.push_front(adjustedDate);
			    }
		    }
		    else
		    {
			    if (rollConventionEnum == ROLLCONV_ENUM_LUNAR)
			    {
				    int days = 0;
				    if (data_frequency == MONTHLY)
				    {
					    days = -28;
				    }
				    else if (data_frequency == QUARTERLY)
				    {
					    days = -91;
				    }
				    else if (data_frequency == SEMI_ANNUAL)
				    {
					    days = -182;
				    }
				    else if (data_frequency == ANNUAL)
				    {
					    days = -364; //=13*28
				    }
				    else
				    {
					    AQ_THROW("Invalid Frequency: Must be Annual, Semi-Annual, Quarterly or Monthly, however '" + data_frequency + "' provided");
				    }
				    unadjustedDate.addDays(days);
				    for (unsigned int i = 1; unadjustedDate > start; i++)
				    {
					    unadjustedDate = end; 
					    unadjustedDate.addDays(i * days);
						if ( unadjustedDate <= start ) break;

					    adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
					    resultDates.push_front(adjustedDate);
				    }
			    }
			    else
			    {
				    int months = 0;
				    if (data_frequency == MONTHLY)
				    {
					    months = -1;
				    }
				    else if (data_frequency == QUARTERLY)
				    {
					    months = -3;
				    }
				    else if (data_frequency == SEMI_ANNUAL)
				    {
					    months = -6;
				    }
				    else if (data_frequency == ANNUAL)
				    {
					    months = -12;
				    }
				    else
				    {
					    AQ_THROW("Invalid Frequency: Must be Annual, Semi-Annual, Quarterly, however '" + data_frequency + "' provided")	
				    }
				    unadjustedDate.addMonths(months);
				    for (unsigned int i = 1; unadjustedDate > start; i++)
				    {
						// apply roll convention
					    unadjustedDate = end; 
					    unadjustedDate.addMonths(i * months);
						setDate(rollDay, unadjustedDate, rollConventionEnum);
					    if ( unadjustedDate <= start ) break;

						adjustedDate = getAdjDate(busDayAdj, calendar, unadjustedDate);
					    resultDates.push_front(adjustedDate);
					    
						// unadjustedDate = end; 
					    // unadjustedDate.addMonths(i * months + months);
					    // setDate(rollDay, unadjustedDate, rollConventionEnum);
				    }
			    }
		    }
	    }
	    
		// Add Start Date if we have a Front Stub
		// **************************************
		AQLDate startRollDate = getAdjDate(busDayAdj, calendar, start);
		if( !inArrears && ( ( resultDates.size() != 0 && startRollDate != resultDates.front() ) || resultDates.size() == 0 ) )
		{
			resultDates.push_front(startRollDate);
		}

		// Add End Date if we have a Back Stub
		AQLDate EndRollDate = getAdjDate(busDayAdj, calendar, end);
		if (inArrears && ((resultDates.size() != 0 && EndRollDate != resultDates.back()) || resultDates.size() == 0))
		{
		    resultDates.push_back(EndRollDate);
		}

		// Convert ResultDates from std::deque to DateVector
	    DateVector out( resultDates.begin(), resultDates.end() );
	    return out;
    }

    /*!
	    @brief calculate stub schedule dates
        @param[in] unadjustedStart	unadjsuted start date
	    @param[in] unadjustedEnd	unadjusted end date
	    @param[in] data_frequency	payment frequency
	    @param[in] inArrears		arrear or not(bool)
	    @param[in] firstStubDate	pointer of front oddday
	    @param[in] lastStubDate		pointer of last oddday
	    @param[in] rollDay			pointer of payment day
	    @param[in] busDayAdj		pointer of SlidingRule
	    @param[in] calendar			pointer of Calender
    */
    void							
    AQLDateHelpers::generateSchedule( const AQLDate& unadjustedStart,
									 const AQLDate& unadjustedEnd,
									 const AQLString& data_frequency,
									 const bool inArrears,
									 const AQLDate* firstStubDate,
									 const AQLDate* lastStubDate,
									 const int* rollDay,
									 DateVector& out,
									 const AQLPriceDataSlidingRule* busDayAdj,
									 const AQLPriceDataCalendar* calendar,
									 const bool isStartRoll,
									 const AQLString* roll_convention )
    {
	    out.clear();
	    AQLDate rollDate; 
	    AQLString freq_str = data_frequency;
	    freq_str.toUpper();

		AQ_THROW_IF( unadjustedStart > unadjustedEnd, "Invalid Schedule: Start Date '" + unadjustedStart.stringWithFormat("DD-MMM-YY") + "' cannot be after the End Date '" + unadjustedEnd.stringWithFormat("DD-MMM-YY") +"'" ) 

	    if (freq_str == NONE)
	    {	
		    rollDate = inArrears ? unadjustedEnd : unadjustedStart;
		    rollDate = getAdjDate(busDayAdj, calendar, rollDate); 
		    out.push_back(rollDate);
		    return;
	    }

	    if (unadjustedStart == unadjustedEnd)
	    {
		    rollDate = getAdjDate(busDayAdj, calendar, unadjustedStart);
		    out.push_back(rollDate);
		    return;
	    }
	
	    const AQLDate& frontStub = ( firstStubDate == NULL && lastStubDate != NULL ) ? unadjustedStart : *firstStubDate; 
	    const AQLDate& endStub	= ( firstStubDate != NULL && lastStubDate == NULL ) ? unadjustedEnd	: *lastStubDate;


	    // *** STUB SCHEDULE USING STUB TYPE ***
	    if ((firstStubDate == NULL && lastStubDate == NULL) || freq_str == BUSINESS_DAYS || freq_str == DAILY || freq_str == WEEKLY)
		{
		    out = generateRegularSchedule(unadjustedStart, unadjustedEnd, freq_str, inArrears, rollDay, busDayAdj, calendar, isStartRoll, roll_convention );
		}
		// *** STUB SCHEDULE USING STUB DATES ***
	    else
	    {
		    out = generateRegularSchedule(frontStub, endStub, freq_str, inArrears, rollDay, busDayAdj, calendar, isStartRoll, roll_convention );

			// Convert the Start and End Dates to Adjusted Dates Here ...
			// ... Since Start and End Dates are Unadjusted to Get the Correct Roll Dates
			// *********************************************************************************

			AQLDate adjustedStartDate = unadjustedStart;
			AQLDate adjustedEndDate = unadjustedEnd;

			if( inArrears )
			{
				// *** Adjusted Start Date ***
				if( unadjustedStart != frontStub )
				{
					adjustedStartDate = getAdjDate(busDayAdj, calendar, frontStub); 
					if (adjustedStartDate != out.front()) out.insert(out.begin(), adjustedStartDate);
				}
				
				// *** Adjusted End Date ***
				if( unadjustedEnd != endStub )
				{
					adjustedEndDate = getAdjDate(busDayAdj, calendar, unadjustedEnd);
					if (adjustedEndDate != out.back())	out.push_back(adjustedEndDate);
				}
			}
			else
			{
				// *** Adjusted Start Date ***
				if( unadjustedStart != frontStub )
				{
					adjustedStartDate = getAdjDate(busDayAdj, calendar, unadjustedStart); 
					if (adjustedStartDate != out.front()) out.insert(out.begin(), adjustedStartDate);
				}

				// *** Adjusted End Date ***
				if( unadjustedEnd != endStub )
				{
					adjustedEndDate = getAdjDate(busDayAdj, calendar, endStub);
					if (adjustedEndDate != out.back()) out.push_back(adjustedEndDate);
				}
			}
	    }			
    }	

    /*!
	    @brief Set term from string to integer 
	
	    @param[in] term	term by string (ex.3Y2M14D,4M5d,11m,,,)
	
	    YMD of string are allowed capital letters and small letters
	    YMD of string need at least one character
	
    */

    void
    AQLDateHelpers::termStrtoYMDW(const AQLString& term, int& y, int& m, int& d, int& w)
    {
		// For backwards compatibility we maintain the old API, and forward to the enhanced function with a dummy calendar days output variable set to zero.
		int c = 0;
		termStrtoYMDWC( term, y, m, d, w, c );
    }

	// ! Set term from string to integer - Enhanced version of the above which handles calendar days, c
	void
	AQLDateHelpers::termStrtoYMDWC(const AQLString& term, int& y, int& m, int& d, int& w, int& c)
	{
		int      pl_y, pl_m, pl_d, pl_w, pl_c;		//place of year, month, busday, week, caledarday
		AQLString str = term;
		str.toUpper();

		if (str == ON)
		{
			str = "1D";
		}
		else if (str == TN)
		{
			str = "2D";
		}
		else if (str == SN)
		{
			str = "3D";
		}

		//find place of...
		pl_y = pl_m = pl_d = pl_w = -1;
		pl_y = str.findString('Y');
		pl_m = str.findString('M');
		pl_d = str.findString('D');
		pl_w = str.findString('W');
		pl_c = str.findString('C');

		if ( pl_d != -1 && pl_c != -1 )
		{
			AQLString msg = "Invalid Tenor String: Cannot have calendar (C) and business-day (D) tenors at the same time";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		if (pl_y == -1 && pl_m == -1 && pl_d == -1 && pl_w == -1 && pl_c == -1 )
		{
			//error
			AQLString msg = "Invalid Tenor String: Term must be Y, M, W, D, C or TN, ON.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// week 
		if ((pl_y != -1 || pl_m != -1 || pl_d != -1 || pl_c != -1) && pl_w != -1)
		{
			//error
			AQLString msg = "#Error: Invalid Tenor String: Weekly Terms W cannot be used in in combination with other term strings.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// String can only contain the following characters and numbers
		// Valid Letters include Y, M, D, W, the minus sign '-' and digits
		// --------------------------------------------------------------------------------

		std::string tempString = str.getCString();
		if (std::string::npos != tempString.find_first_not_of("YMDWC-0123456789"))
		{
			std::string msg = "Invalid Tenor String '" + tempString + "' : Term must be Y, M, W, D, C or TN, ON and contain no spaces";
			throw AQLCoreInvalidData(msg.c_str(), __FILE__, __LINE__);
		}

		// --------------------------------------------------------------------------------

		// Initialise return values
		y = 0;
		m = 0;
		d = 0;
		w = 0;
		c = 0;

		// Insert into map in order to sort by position
		std::map<int, char> mp;
		if (pl_y != -1) mp.insert(make_pair(pl_y, 'Y'));
		if (pl_m != -1) mp.insert(make_pair(pl_m, 'M'));
		if (pl_d != -1) mp.insert(make_pair(pl_d, 'D'));
		if (pl_w != -1) mp.insert(make_pair(pl_w, 'W'));
		if (pl_c != -1) mp.insert(make_pair(pl_c, 'C'));

		int currentPosition = 0;
		std::map<int, char>::iterator it = mp.begin();

		while (it != mp.end())
		{
			int endPosition = it->first;
			switch (it->second)
			{
			case 'Y':
				y = str.subString(currentPosition, endPosition).getIntValue();
				break;

			case 'M':
				m = str.subString(currentPosition, endPosition).getIntValue();
				break;

			case 'D':
				d = str.subString(currentPosition, endPosition).getIntValue();
				break;

			case 'W':
				w = str.subString(currentPosition, endPosition).getIntValue();
				break;

			case 'C':
				c = str.subString(currentPosition, endPosition).getIntValue();
				break;
			}
			currentPosition = endPosition + 1;
			++it;
		}
	}

    /*!
	    @brief get date either term after or before from basedate and roll (day is actual day)
	
	    @param[in] basedate			base date
	    @param[in] term				stirng of term (D is actual day)
	    @param[in] rollForwards			true:after, false:before(bool)
	    @param[in] roll_conv		roll convention
	
    */
    AQLDate
    AQLDateHelpers::getDate(const AQLDate& basedate, 
                                 const AQLString& term, 
                                 bool rollForwards, 
                                 const AQLString* roll_conv)
    {
	    int y, m, d, w;
	    AQLDateHelpers::termStrtoYMDW(term, y, m, d, w);
	    AQLDate date = basedate;
	    if (!rollForwards)
	    {
		    y = -y;
		    m = -m;
		    d = -d;
		    w = -w;
	    }
        
        date.addYears(y);
		date.addMonths(m);
        
        date.addWeeks(w);
	    date.addDays(d);

	    if (roll_conv) date = rollDate(date, roll_conv);
	
	    return date;
    }

    /*!
	    @brief get date, roll and slide (if roll convention is set, day is actual day, otherwise business day.)	
	
	    @param[in] basedate			base date
	    @param[in] term				stirng of term (D is business day)
	    @param[in] busdayrule		SlidingRule
	    @param[in] pCal				pointer of Calender
	    @param[in] rollForwards			true:after, false:before(bool)
	    @param[in] roll_conv		roll convention
    */
    AQLDate AQLDateHelpers::getDate(const AQLDate& basedate, 
                                  const AQLString& term, 
                                  const AQLPriceDataSlidingRule& busdayrule,
                                  const AQLPriceDataCalendar* pCal,
                                  bool rollForwards,
							      const AQLString* roll_conv)
    {
		// These variables store the number of years, months, busdays, weeks, and calendardays represented by "term"
	    int y, m, d, w, c;
	    AQLDateHelpers::termStrtoYMDWC(term, y, m, d, w, c);
	    if (!rollForwards)
	    {
		    y = -y;
		    m = -m;
		    d = -d;
		    w = -w;
			c = -c;
	    }
	    AQLDate date = basedate;
	    
        // set W only
	    if (w != 0) // cannot use W with other terms
	    {
		    date.addWeeks(w);
		    date = slideDate(date, busdayrule, pCal);
		    return date;
	    }

	    // set Y And/Or M (with D)
		// If we have a year or month, then enter this if statement to addYears()/addMonths(), and make a businessday adjustment.
		// if we have a business-day or calendar-day, do not enter this block, but instead handle the days later.
	    if (y != 0 || m != 0 || (d == 0 && c == 0) )
	    {
            date.addYears(y);
			date.addMonths(m);
			if (roll_conv)
            {
                date = rollDate(date, roll_conv);
            }

		    date = slideDate(date, busdayrule, pCal);
	    }

	    // set D and C
	    if (d == 0 && c == 0) return date;

		if ( c != 0 )	// Calendar day case
		{
			date.addDays(c);
			date = slideDate(date, busdayrule, pCal);
		}
	    else if (!roll_conv && busdayrule.getSlidingRule() != SLIDING_RULE_NO_CHANGE) // business day case
	    {
		    if (!pCal)
		    {
                AQLString msg = "#Error: Missing Calendar";
			    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		    }
		    date = pCal->getBusinessDay(date, d);
	    }
	    else // calendar days plus final adjustment, in the case where roll convention is specified or NO_CHANGE is specified
	    {
		    date.addDays(d);
		    date = slideDate(date, busdayrule, pCal);
	    }
	    return date;
    }

    /*!
	    @brief slide date

	    @param[in] date		basedate
	    @param[in] srule	sliding rule
	    @param[in] pCal		pointer of Calendar
	    @return				date
    */
    AQLDate 
    AQLDateHelpers::slideDate(const AQLDate& date, const AQLPriceDataSlidingRule& srule, const AQLPriceDataCalendar* pCal)
    {
	    if (srule.getSlidingRule() != SLIDING_RULE_NO_CHANGE) 
	    {
		    AQ_THROW_IF(!pCal, "Missing Calendar")
		    return srule.getDate(date, *pCal);
	    }
	    return date;
    }

    /*!
	    @brief roll date

	    @param[in] date		basedate
	    @param[in] roll		roll convention
	    @return				date
    */
    AQLDate 
    AQLDateHelpers::rollDate(const AQLDate& basedate, const AQLString* roll)
    {
	    AQLDate date = basedate;
	    const int day = date.dayOfMonth();

	    const RollConventionEnum rollConventionEnum = roll == NULL ? ROLLCONV_ENUM_NONE : toRollConventionEnum( *roll );

	    setDate(&day, date, rollConventionEnum);
	    return date;
    }

    /*!
	    @brief get IMM date (the third Wednesday of March, June, September and December)

	    @param[in] y				year
	    @param[in] m				month	
    */
    AQLDate
    AQLDateHelpers::getIMMDate(const int& y, const int& m, bool isOddMonth)
    {
	    if ((m%3 && !isOddMonth) || m <= 0 || m>=13) 
            throw AQLCoreInvalidData("#Error: Invalid IMM Month: IMM Month must be 3, 6, 9  or 12", __FILE__, __LINE__);
	
	    AQLDate date;
	    date.setYear(y); date.setMonth(m); date.setDay(1);
	    AQLDayOfWeekEnum weekly = date.dayOfWeek();
	    if (weekly == SUN)
        {
            date.addDays(3);
        }
        else if (weekly == MON)
        {
            date.addDays(2);
        }
        else if (weekly == TUE)
        {
            date.addDays(1);
        }
        else if (weekly == WED)
        {
            date.addDays(0);
        }
        else if (weekly == THU)
        {
            date.addDays(6);
        }
        else if (weekly == FRI)
        {
            date.addDays(5);
        }
        else if (weekly == SAT)
        {
            date.addDays(4);
        }
        else 
        {
            AQLString    msg = "#Error: Invalid Date: Invalid Weekday [";
            msg += weekly + "]";
            throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
	
	    date.addWeeks(2);

	    return date;
    }

    /*!
	    @brief get IMM date (the 20th day of March, June, September and December)

	    @param[in] startDate		start date
	    @param[in] strTerm		    stirng of term
	    @param[in] lag				lag
    */
    AQLDate 
    AQLDateHelpers::getImmEndDate(const AQLDate& startDate, const AQLString& strTerm, int lag)
    {
	    AQLDate	endDate = getDate(startDate, strTerm, true);

	    // set year YYYY
	    AQLDate	s1(endDate);
	    AQLDate	e1(endDate), e2(endDate), e3(endDate), e4(endDate);

	    // s1 : YYYY/1/1
	    s1.setMonth(1);
	    s1.setDay(1);

	    // e1 : YYYY/3/20
	    e1.setMonth(3);
	    e1.setDay(20);

	    // e2 : YYYY/6/20
	    e2.setMonth(6);
	    e2.setDay(20);

	    // e3 : YYYY/9/20
	    e3.setMonth(9);
	    e3.setDay(20);

	    // e4 : YYYY/12/20
	    e4.setMonth(12);
	    e4.setDay(20);

	    if (s1 <= endDate && endDate < e1)
	    {
		    endDate = e1;
	    }
	    else if (e1 <= endDate && endDate < e2)
	    {
		    endDate = e2;
	    }
	    else if (e2 <= endDate && endDate < e3)
	    {
		    endDate = e3;
	    }
	    else if (e3 <= endDate && endDate < e4)
	    {
		    endDate = e4;
	    }
	    else
	    {
		    endDate = e1;
		    endDate.addYears(1);
	    }

	    return endDate;
    }

    /*!
	    @brief get futures contract start date

	    @param[in]      month   futures contract month
	    @param[in]      year    futures contract year
        @param[out]     returns the futures contract start date
    */
    AQLDate
    AQLDateHelpers::getFuturesContractStartDate(const unsigned int& month, const unsigned int& year )
    {
	    if ( month > 12 ) 
        {
            throw AQLCoreInvalidData("#Error: Invalid futures contract month.", __FILE__, __LINE__);
        }
	
	    AQLDate date;
	    date.setYear( year );
        date.setMonth( month );
        date.setDay( 1 );
	
        AQLDayOfWeekEnum weekly = date.dayOfWeek();
	    if (weekly == SUN)
        {
            date.addDays(3);
        }
        else if (weekly == MON)
        {
            date.addDays(2);
        }
        else if (weekly == TUE)
        {
            date.addDays(1);
        }
        else if (weekly == WED)
        {
            date.addDays(0);
        }
        else if (weekly == THU)
        {
            date.addDays(6);
        }
        else if (weekly == FRI)
        {
            date.addDays(5);
        }
        else if (weekly == SAT)
        {
            date.addDays(4);
        }
        else 
        {
            throw AQLCoreInvalidData("#Error: Invalid day of the week.", __FILE__, __LINE__);
        }
	
	    date.addWeeks( 2 );

	    return date;
    }

    /*!
	    @brief get IMM date from future term
	
	    @param[in] baseDate				asOfdate of market rate
	    @param[in] futureTerm			term of future market(ex. EDV1,EDV2)
    */
    AQLDate
    AQLDateHelpers::getIMMDateFromTerm(const AQLDate& baseDate, const AQLString& futureTerm)
    {
	    if (futureTerm.size() != 4) 
            throw AQLCoreInvalidData("#Error: Invalid Futures Contract Tenor - Futures contracts must have 4 characters", __FILE__, __LINE__);

	    if (!isdigit(*futureTerm.subString(3, 3).getCString()))
		    throw AQLCoreInvalidData("#Error: Invalid Futures Contract Tenor - Futures contracts must end with a digit", __FILE__, __LINE__);
	    
        unsigned int month = changeFutureMonthFormat(futureTerm.subString(2, 2));
	    int year = baseDate.yearOfEra();
	    while (year % 10 != futureTerm.subString(3, 3).getIntValue())
	    {
		    ++year;
	    }
	    return getIMMDate(year, month, true);
    }


    /*!
	    @brief get FF dates from FF term
	
	    @param[in] baseDate				asOfdate of market rate
	    @param[in] futureTerm			term of future market(ex. EDV1,EDV2)
    */
    DateVector
    AQLDateHelpers::getFFDatesFromTerm(const AQLDate& baseDate, const AQLString& fedfundTerm)
    {
	    if (fedfundTerm.size() != 4) 
            throw AQLCoreInvalidData("#Error: Invalid FED Fund Contract Tenor - Contract tenor must be 4 characters", __FILE__, __LINE__);
	    
        unsigned int month = changeFutureMonthFormat(fedfundTerm.subString(2, 2));
	    AQLString yearStr_basedate = AQLString(baseDate.yearOfEra());
	    AQLString yearStr = yearStr_basedate.subString(0, 2) + fedfundTerm.subString(3, 3);

	    AQLDate startdate;
	    startdate.setYear(yearStr.getIntValue());
	    startdate.setMonth(month);
	    startdate.setDay(1);

	    AQLDate enddate = startdate;
	    enddate.addMonths(1);
	    enddate.addDays(-1);

	    DateVector ret(2);
	    ret[0] = startdate;
	    ret[1] = enddate;

	    return ret;
    }


    /*!
	    @brief get FXSpotDate date
	
	    @param[in] baseDate				asOfdate of market rate
	    @param[in] keyFX				FX identify in String (ex. "JPY/USD")
	    @param[in] spotlag				spot lag in int (ex. 2)
	    @param[in] calStr				city of calendar in String (ex. "TkB:LnB")
	    @param[in] rollForwards				true:after, false:before(bool)
    */
    AQLDate
    AQLDateHelpers::getFXSpotDate(const AQLString& keyFX,
								       const AQLDate& basedate,
								       const AQLString& calStr,
								       int spotlag,
								       bool rollForwards)
    {
	    AQLDate ret;
	    AQLString tempStr = calStr;
	    tempStr.toUpper();

	    int nybpos = tempStr.findString("NYB");
	    if (nybpos < 0) {
		    SlidingRuleType sruleType;
		    sruleType = (spotlag >= 0) ? SLIDING_RULE_FOLLOWING : SLIDING_RULE_PRECEDING;
		    AQLPriceDataSlidingRule busdayrule(sruleType);

		    AQLPriceDataCalendar Cal;
		    Cal.convertFromString(calStr);

		    ret = getDate(basedate, AQLString(spotlag) + "D", busdayrule, &Cal, rollForwards);
	    } else {
		    // Remove NYB Calendar
		    AQLStringVector tempStrs(tempStr.toToken(':'));
		    tempStr = "";
		    for(AQLStringVector::iterator it = tempStrs.begin(); it != tempStrs.end(); it++)
			    if(*it != "NYB")
				    tempStr += *it + ":";

		    // Remove final ":"
		    if(tempStr.size() > 0)
			    tempStr.remove(tempStr.size() - 1, 1);
	
		    AQLString tempStr2 = keyFX;
		    tempStr2.toUpper();
		    AQLStringVector curs(tempStr2.toToken('/'));

		    if(curs[0] == "USD")
			    ret = AQLMathFXUtility::getSpotDate_IncludedUSD(curs[0],basedate,tempStr,"NyB",spotlag);
		    else if(curs[1] == "USD")
			    ret = AQLMathFXUtility::getSpotDate_IncludedUSD(curs[1],basedate,tempStr,"NyB",spotlag);
		    else
			    // cross USD FX
			    ret = AQLMathFXUtility::getSpotDate_IncludedUSD(tempStr2,basedate,tempStr,"NyB",spotlag);
	    }
	    return ret;
    }

    /* static */ bool
    AQLDateHelpers::haveNextCBDate(
	    const AQLString& cb, const AQLDate& baseDate, bool strictlyAfter)
    {
	    AQLDate ignore;
	    return getIfExistsNextCBDate(cb, baseDate, strictlyAfter, ignore);
    }

    /* static */ AQLDate
    AQLDateHelpers::getNextCBDate(
	    const AQLString& cb, const AQLDate& baseDate, bool strictlyAfter)
    {
	    AQLDate result;
	    const bool exists = getIfExistsNextCBDate(cb, baseDate, strictlyAfter, result);

	    if (!exists) {
		    if (strictlyAfter) {
                throw AQLCoreAppError("#Error: No central bank data available after specified reference date", __FILE__, __LINE__);
		    }
		    else {
			    throw AQLCoreAppError("#Error: No central bank data available on or after specified reference date", __FILE__, __LINE__);
		    }
	    }

	    return result;
    }

    /* static */ bool
    AQLDateHelpers::getIfExistsNextCBDate(
	    const AQLString& cb, const AQLDate& baseDate, bool strictlyAfter, AQLDate& result)
    {
	    typedef vector<AQLDate> Schedule;
	    typedef Schedule::const_iterator cIter;

	    const Schedule& cbDates = AQLMathCentralBank::meetingSchedule(cb);

	    cIter it = std::lower_bound(cbDates.begin(), cbDates.end(), baseDate);

	    if (it == cbDates.end()) {
		    return false;
	    }

	    if (!strictlyAfter) {
		    result = *it;
		    return true;
	    }

	    assert(strictlyAfter);
	
	    if (*it == baseDate) {
		    // baseDate is an central bank schedule date; we're in strict mode: move forward
		    if (++it == cbDates.end()) {
			    return false;
		    }
	    }

	    result = *it;
	    return true;
    }

    /*!
	    @brief get month corresponding to string of future market format
	
	    @param[in] futureMonth			string of future market format(ex. V,Z,K)
    */
    unsigned int
    AQLDateHelpers::changeFutureMonthFormat(const AQLString& futureMonth)
    {
        if (futureMonth == F_FUTURE_MONTH)
        {
            return 1;
	    }
	    else if (futureMonth == G_FUTURE_MONTH)
	    {
            return 2;
	    }
	    else if (futureMonth == H_FUTURE_MONTH)
	    {
            return 3;
	    }
	    else if (futureMonth == J_FUTURE_MONTH)
	    {
            return 4;
	    }
	    else if (futureMonth == K_FUTURE_MONTH)
	    {
            return 5;
	    }
	    else if (futureMonth == M_FUTURE_MONTH)
	    {
            return 6;
	    }
	    else if (futureMonth == N_FUTURE_MONTH)
	    {
            return 7;
	    }
	    else if (futureMonth == Q_FUTURE_MONTH)
	    {
            return 8;
	    }
	    else if (futureMonth == U_FUTURE_MONTH)
	    {
            return 9;
	    }
	    else if (futureMonth == V_FUTURE_MONTH)
	    {
            return 10;
	    }
	    else if (futureMonth == X_FUTURE_MONTH)
	    {
            return 11;
	    }
	    else if (futureMonth == Z_FUTURE_MONTH)
	    {
            return 12;
	    }
	    else
	    {
            throw AQLCoreInvalidData("#Error: Invalid Futures Contract: Futures month is invalid", __FILE__, __LINE__);
	    }	
    }


    /*!
        @brief return convert from double grid to dates grid using ACT/365 *** Use for smooth date grids - ACT/365_ISDA causes leap year irrgularities ***

	    @param[in] asofDate
	    @param[in] terms
	    @param[out] dates
	
    */
    void
    AQLDateHelpers::convertToDateGrid(const AQLDate &asofDate, const DoubleArray &terms, DateVector &dates)
    {
	    dates.clear();
	
	    const AQLPriceDataDayCount dc_act365(ACT_365);

	    for (unsigned int i = 0; i < terms.size(); ++i)
	    {
		    const double term = terms[i];

		    AQLDate date = AQLDateScheduleHelpers::getDateFromTerm( asofDate, term, dc_act365 );

		    dates.push_back(date);
	    }

	    if (dates.size() != terms.size())
	    {
            throw AQLCoreInvalidData("#Error: Inconsistent Data: Number of dates and terms must match", __FILE__, __LINE__);
	    }
    }

    // Return the frequency in months for comparing two frequencies, not for accurate calculations
	// *** Duplicate method in AQLMathDateCalculations.cpp ***
    double AQLDateHelpers::getPeriodFrequencyInMonths(const AQLString& freq)
    {
	    double frequencyInMonths;

		AQLString freq_capital = freq;
		freq_capital.toUpper();

		if (freq_capital == ANNUAL)											frequencyInMonths = 12.0;
		else if (freq_capital == SEMI_ANNUAL)								frequencyInMonths = 6.0;
		else if (freq_capital == QUARTERLY)									frequencyInMonths = 3.0;
		else if (freq_capital == MONTHLY || freq_capital == LUNAR)			frequencyInMonths = 1.0;
		else if (freq_capital == WEEKLY )									frequencyInMonths = 0.25;			// 1/4 of a month
		else if (freq_capital == DAILY || freq_capital == BUSINESS_DAYS)	frequencyInMonths = 0.0333333333;	// 1/30 of a months
		else
		{
			AQ_THROW("Invalid Frequency: Must be ANNUAL, SEMI_ANNUAL, QUARTERLY, MONTHLY, WEEKLY or DAILY");
		}

		return frequencyInMonths;
    }

    /*!
        @brief calc compounding times

	    @param[in] freq_rst	: frequency of reset
	    @param[in] freq_pay : frequency of payment

	    @return compounding times
    */
    int
    AQLDateHelpers::calcCompoundingTimes(const AQLString& freq_rst, const AQLString& freq_pay)
    {
	    int span_rst = getPeriodFrequencyInMonths(freq_rst);
	    int	span_pay = getPeriodFrequencyInMonths(freq_pay); 

	    if (span_pay < span_rst) //error
            throw AQLCoreInvalidData("#Error: Invalid compound frequency - Pay frequency cannot be more regular than the accrual / reset frequency", __FILE__, __LINE__);
	    else return span_pay / span_rst;
    }

    /* static */
    AQLDate AQLDateHelpers::getNextWeekdayDate(
	    AQLDayOfWeekEnum weekday, const AQLDate& baseDate, bool strictlyAfter)
    {
	    AQLDate result(baseDate);

		// Get the current day of the week and check it is valid
	    const unsigned int currentDayOfWeek = static_cast<unsigned int>( baseDate.dayOfWeek() );
	    assert(currentDayOfWeek < 7);
	    
		// Get the target day of the week and check it is valid
		const unsigned int targetDayOfWeek  = static_cast<unsigned int>( weekday );
	    assert(targetDayOfWeek < 7);
	    
		// Evaluate the difference between the current and target day of week
		const unsigned int difference = currentDayOfWeek <= targetDayOfWeek ? targetDayOfWeek - currentDayOfWeek : targetDayOfWeek + 7 - currentDayOfWeek;
	    assert(0 <= difference && difference < 7);

		// Apply the difference
		result.addDays(difference == 0 && strictlyAfter ? 7 : difference);
		assert(result.dayOfWeek() == weekday);

	    return result;
    }

	//
	// Static - Nth ECB Meeting Date
	// Note: StrictlyAfter = roll the ECB date if it falls on the asOfDate
	//
	AQLDate AQLDateHelpers::getNthECBMeetingDate(const AQLDate& asOfDate, const int n, bool strictlyAfter)
	{
		AQ_REQUIRE(n > 0, "ECB Meeting Date - Invalid Input: n must be larger than zero");
		
		// n=1 case, note strictlyAfter = false
		AQLDate ecbMeetingDate = AQLDateHelpers::getNextECBDate(asOfDate, strictlyAfter);
		
		// n=2 onwards, note strictlyAfter = true
		for (size_t i = 0; i < size_t(n-1); ++i) 
		{
			ecbMeetingDate = AQLDateHelpers::getNextECBDate(ecbMeetingDate, true);
		}

		return ecbMeetingDate;
	}

	//
	// Static - Nth ECB Swap Start Date
	// Note: StrictlyAfter = roll the ECB date if it falls on the asOfDate
	//
	AQLDate AQLDateHelpers::getNthECBSwapStartDate(const AQLDate& asOfDate, const int n, bool strictlyAfter)
	{
		AQLDate ecbMeetingDate = getNthECBMeetingDate(asOfDate,n,strictlyAfter);
		return AQLDateHelpers::getECBStartDate(ecbMeetingDate); // Always Wednesday
	}

	//
	// Static - Nth ECB Swap End Date
	// Note: StrictlyAfter = roll the ECB date if it falls on the asOfDate
	//
	AQLDate AQLDateHelpers::getNthECBSwapEndDate(const AQLDate& asOfDate, const int n, bool strictlyAfter)
	{
		// Nth Swap End Date = (N+1)th Swap Start Date
		AQLDate ecbSwapEndDate = getNthECBSwapStartDate(asOfDate, n + 1, strictlyAfter);
		ecbSwapEndDate.addDays(-1);
		return ecbSwapEndDate; //Always Tuesday
	}

	//
	// Static - Next ECB Meeting Date
	// Note: StrictlyAfter = roll the ECB date if it falls on the inputDate
	//
	AQLDate AQLDateHelpers::getNextECBMeetingDate(const AQLDate& meetingDate)
	{
		return getNextECBDate(meetingDate,true); // StrictlyAfter = true
	}

	//
	// Static - Next ECB Swap Start Date
	// Note: StrictlyAfter = roll the ECB date if it falls on the inputDate
	//
	AQLDate AQLDateHelpers::getNextECBSwapStartDate(const AQLDate& swapStartDate)
	{
		return getNthECBSwapStartDate(swapStartDate,1); // nth date = 1
	}

	//
	// Static - Next ECB Swap Start Date
	// Note: StrictlyAfter = roll the ECB date if it falls on the inputDate
	//
	AQLDate AQLDateHelpers::getNextECBSwapEndDate(const AQLDate& swapEndDate)
	{
		// getNthECBSwapEndDate takes asOfDate as input not SwapEndDate, hence we use n=0 (not n=1) with strictlyAfter = true
		AQLDate ecbSwapEndDate = getNthECBSwapEndDate(swapEndDate, 0, true);
		return ecbSwapEndDate; //Always Tuesday
	}
}
