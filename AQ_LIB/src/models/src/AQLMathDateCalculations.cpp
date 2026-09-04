/*! @file
    @brief source code of utility class to calculate dates of schedule.
*/
///

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathDateCalculations.h"
#include "AQLMathDateUtilities.h"

#include <algorithm>
#include <cmath>
#include <cctype>						// isdigit - checks if character is numerical 0-9
#include <cassert>
#include <boost/algorithm/string.hpp>   // boost::iequals

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

using namespace std;

#define	BUSINESS_DAYS			"BUSINESS_DAYS"
#define DAILY					"DAILY"
#define	WEEKLY					"WEEKLY"
#define	NONE					"NONE"
#define ANNUAL					"ANNUAL"
#define SEMI_ANNUAL				"SEMI-ANNUAL"
#define QUARTERLY				"QUARTERLY"
#define MONTHLY					"MONTHLY"
#define LUNAR					"LUNAR"

#define ON						"ON"
#define TN						"TN"
#define SN						"SN"

#define F_FUTURE_MONTH			"F"
#define G_FUTURE_MONTH			"G"
#define H_FUTURE_MONTH			"H"
#define J_FUTURE_MONTH			"J"
#define K_FUTURE_MONTH			"K"
#define M_FUTURE_MONTH			"M"
#define N_FUTURE_MONTH			"N"
#define Q_FUTURE_MONTH			"Q"
#define U_FUTURE_MONTH			"U"
#define V_FUTURE_MONTH			"V"
#define X_FUTURE_MONTH			"X"
#define Z_FUTURE_MONTH			"Z"

#define ROLLCONV_IMM			"IMM"
#define ROLLCONV_NORMAL			"NORMAL"
#define ROLLCONV_STARTDATE		"STARTDATE"
#define ROLLCONV_ENDDATE		"ENDDATE"
#define ROLLCONV_EOM			"EOM"
#define ROLLCONV_LUNAR			"LUNAR"


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
			AQ_THROW("Invalid Roll Convention: Must be NORMAL, IMM or EOM")
		}
	}
}

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
			AQ_THROW("Invalid Roll Convention: Must be NORMAL, IMM or EOM")
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


/*!
	@brief get holiday adjust date
*/
AQLDate
AQLMathDateCalculations::getAdjDate(const AQLPriceDataSlidingRule* pbusdayrule,
								const AQLPriceDataCalendar* pcal, const AQLDate &date)
{
	if (pbusdayrule != NULL && pcal != NULL)
	{
		return pbusdayrule->getDate(date, *pcal);
	}
	else
	{
		return date;
	}
}


void
AQLMathDateCalculations::setDate(const int* pday, AQLDate &date, const RollConventionEnum rollConventionEnum)
{
	if ( rollConventionEnum == ROLLCONV_ENUM_NONE ||
		 rollConventionEnum == ROLLCONV_ENUM_NORMAL ||
		 rollConventionEnum == ROLLCONV_ENUM_STARTDATE ||
		 rollConventionEnum == ROLLCONV_ENUM_ENDDATE ||
		 rollConventionEnum == ROLLCONV_ENUM_LUNAR )
	{
		if(pday == NULL) return;
		date.setDay(min(*pday, (int)LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(date.yearOfEra())][0][date.monthOfYear() - 1]));
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
	@brief set date
*/


/*!
	@brief calculate dates based basic pattern

    @param[in] start			start date
	@param[in] end				end date
	@param[in] data_frequency	payment frequency
	@param[in] isarrear			arrear or not(bool)
	@param[in] pday				pointer of payment day
	@param[in] pbusdayrule		pointer of SlidingRule
	@param[in] pcal				pointer of Calender

    @return DateVector of grid points
*/

DateVector
AQLMathDateCalculations::generateRegularSchedule(const AQLDate& start, 
									 const AQLDate& end,
									 const AQLString& data_frequency,
									 const bool isarrear,
									 const int* pday,
									 const AQLPriceDataSlidingRule* pbusdayrule,
									 const AQLPriceDataCalendar* pcal,
									 const bool isstartroll,
									 const AQLString* roll_convention)
{

	std::deque<AQLDate> dates;

	AQLDate tempDate;
	AQLDate sldDate;

	RollConventionEnum rollConventionEnum = roll_convention == NULL ? ROLLCONV_ENUM_NONE : toRollConventionEnum( *roll_convention );

	if (isstartroll)
	{
		tempDate = start;
		if (data_frequency == BUSINESS_DAYS || data_frequency == DAILY)
		{
			tempDate.addDays(1);

			for ( ;tempDate < end ;tempDate.addDays(1))
			{
				sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
				if (tempDate == sldDate)
					dates.push_back(sldDate);
			}
		}
		else if (data_frequency == WEEKLY)
		{
			tempDate.addDays(7);

			for ( ;tempDate < end ;tempDate.addDays(7))
			{
				sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
				dates.push_back(sldDate);
			}
		}
		else if (data_frequency == LUNAR)
		{
			tempDate.addDays(28);

			for ( ;tempDate < end ;tempDate.addDays(28))
			{
				sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
				dates.push_back(sldDate);
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
					AQ_THROW("Invalid Frequency: Must be ANNUAL, SEMI-ANNUAL, QUARTERLY or MONTHLY.")
				}
				tempDate.addDays(days);
				for (unsigned int i = 1; tempDate < end; i++)
				{
					tempDate = start; 
					tempDate.addDays(i * days);

					sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
					dates.push_back(sldDate);
					tempDate = start; 
					tempDate.addDays(i * days + days);
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
					AQ_THROW("Invalid Frequency: Must be ANNUAL, SEMI-ANNUAL, QUARTERLY or MONTHLY.")		
				}
				tempDate.addMonths(months);
				for (unsigned int i = 1; tempDate < end; i++)
				{
					tempDate = start; 
					tempDate.addMonths(i * months);

					setDate( pday, tempDate, rollConventionEnum );
					sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
					dates.push_back(sldDate);
					tempDate = start; 
					tempDate.addMonths(i * months + months);
					setDate(pday, tempDate, rollConventionEnum);
				}
			}
		}
	}
	else
	{
		AQLDate tempDate = end;
		if (data_frequency == BUSINESS_DAYS || data_frequency == DAILY)
		{
			tempDate.addDays(-1);

			for ( ;tempDate > start ;tempDate.addDays(-1))
			{
				sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
				if (tempDate == sldDate)
					dates.push_front(sldDate);
			}
		}
		else if (data_frequency == WEEKLY)
		{
			tempDate.addDays(-7);

			for ( ;tempDate > start ;tempDate.addDays(-7))
			{
				sldDate = getAdjDate(pbusdayrule, pcal, tempDate); 
				dates.push_front(sldDate);
			}
		}
		else if (data_frequency == LUNAR)
		{
			tempDate.addDays(-28);

			for ( ;tempDate > start ;tempDate.addDays(-28))
			{
				sldDate = getAdjDate(pbusdayrule, pcal, tempDate); 
				dates.push_front(sldDate);
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
					AQ_THROW("Invalid Frequency: Must be ANNUAL, SEMI-ANNUAL, QUARTERLY or MONTHLY.")		
				}
				tempDate.addDays(days);
				for (unsigned int i = 1; tempDate > start; i++)
				{
					tempDate = end; 
					tempDate.addDays(i * days);

					sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
					dates.push_front(sldDate);
					tempDate = end; 
					tempDate.addDays(i * days + days);
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
					AQ_THROW("Invalid Frequency: Must be ANNUAL, SEMI-ANNUAL, QUARTERLY or MONTHLY.")		
				}
				tempDate.addMonths(months);
				for (unsigned int i = 1; tempDate > start; i++)
				{
					tempDate = end; 
					tempDate.addMonths(i * months);

					setDate(pday, tempDate, rollConventionEnum);
					sldDate = getAdjDate(pbusdayrule, pcal, tempDate);
					dates.push_front(sldDate);
					tempDate = end; 
					tempDate.addMonths(i * months + months);
					setDate(pday, tempDate, rollConventionEnum);
				}
			}
		}
	}

	sldDate = getAdjDate(pbusdayrule, pcal, start);
	if (!isarrear && ((dates.size() != 0 && sldDate != dates.front()) || dates.size() == 0))
		dates.push_front(sldDate);

	sldDate = getAdjDate(pbusdayrule, pcal, end);
	if (isarrear && ((dates.size() != 0 && sldDate != dates.back()) || dates.size() == 0))
		dates.push_back(sldDate);

	DateVector out( dates.begin(), dates.end() );
	return out;
}

/*!
	@brief calculate dates

    @param[in] start			start date
	@param[in] end				end date
	@param[in] data_frequency	payment frequency
	@param[in] isarrear			arrear or not(bool)
	@param[in] firstStubDate	pointer of front oddday
	@param[in] lastStubDate		pointer of last oddday
	@param[in] pday				pointer of payment day
	@param[in] pbusdayrule		pointer of SlidingRule
	@param[in] pcal				pointer of Calender
*/
void AQLMathDateCalculations::generateSchedule( const AQLDate& start,
											   const AQLDate& end,
											   const AQLString& data_frequency,
											   const bool isarrear,
											   const AQLDate* firstStubDate,
											   const AQLDate* lastStubDate,
											   const int* pday,
											   DateVector& out,
											   const AQLPriceDataSlidingRule* pbusdayrule,
											   const AQLPriceDataCalendar* pcal,
											   const bool isstartroll,
											   const AQLString* roll_convention)
{
	out.clear();
	AQLDate sldDate; 
	AQLString freq_str = data_frequency;
	freq_str.toUpper();

	if (start > end)
	{
		AQ_THROW("Invalid Dates: Start Date '" + start.stringWithFormat("DD-MMM-YY") + "' must be before End Date '" + end.stringWithFormat("DD-MMM-YY") + "'")			
	}

	if (freq_str == NONE)
	{	
		sldDate = isarrear ? end : start;
		sldDate = getAdjDate(pbusdayrule, pcal, sldDate); 
		out.push_back(sldDate);
		return;
	}

	if (start == end)
	{
		sldDate = getAdjDate(pbusdayrule, pcal, start);
		out.push_back(sldDate);
		return;
	}
	
	const AQLDate& fodd = firstStubDate == NULL && lastStubDate != NULL ? start : *firstStubDate; 
	const AQLDate& lodd = firstStubDate != NULL && lastStubDate == NULL ? end   : *lastStubDate;
	
	//check timing of oddday
	if ( (firstStubDate != NULL || lastStubDate != NULL) && (start > fodd || fodd > lodd || lodd > end) )
	{
	    AQ_THROW( "Invalid Cashflow Dates: Cashflow dates must be startDate <= firstStubDate <= lastStubDate <= endDate" )
	}
	
	//basic pattern
	if ((firstStubDate == NULL && lastStubDate == NULL) || freq_str == BUSINESS_DAYS || freq_str == DAILY || freq_str == WEEKLY)
		out = generateRegularSchedule(start, end, freq_str, isarrear, pday, pbusdayrule, pcal, isstartroll, roll_convention);
	//odd pattern
	else
	{
		out = generateRegularSchedule(fodd, lodd, freq_str, isarrear, pday, pbusdayrule, pcal, isstartroll, roll_convention);

		if (!isarrear && start != fodd) 
		{		
			sldDate = getAdjDate(pbusdayrule, pcal, start); 
			if (sldDate != out.front())
                out.insert(out.begin(), sldDate);
		}
		if (isarrear && start != fodd)
		{
			sldDate = getAdjDate(pbusdayrule, pcal, fodd); 
			if (sldDate != out.front())
				out.insert(out.begin(), sldDate);
		}
		if (!isarrear && end != lodd)
		{
			sldDate = getAdjDate(pbusdayrule, pcal, lodd);
			if (sldDate != out.back())
				out.push_back(sldDate);
		}
		if (isarrear && end != lodd)
		{		
			sldDate = getAdjDate(pbusdayrule, pcal, end);
			if (sldDate != out.back())
                out.push_back(sldDate);
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
AQLMathDateCalculations::termStrtoYMDW(const AQLString& term, int& y, int& m, int& d, int& w)
{
	// place of year, month, day, week
    int pl_y, pl_m, pl_d, pl_w;		
    pl_y = pl_m = pl_d = pl_w = -1;

    AQLString str = term;
    str.toUpper();

	if ( boost::iequals( str.getCString(), "ON" ) )
	{
		str = "1D";
	}
	else if ( boost::iequals( str.getCString(), "TN" ) )
	{
		str = "2D";
	}
	else if ( boost::iequals( str.getCString(), "SN" ) )
	{
		str = "3D";
	}

	// find place of...
	pl_y = str.findString('Y');
	pl_m = str.findString('M');
	pl_d = str.findString('D');
	pl_w = str.findString('W');
		
	if (pl_y == -1 && pl_m == -1 && pl_d == -1 && pl_w == -1) 
	{
		AQ_THROW( "Invalid Tenor String: Term must have format ON, TN, Y, M, W or D." )
	}
	// week 
	if ((pl_y != -1 || pl_m != -1 || pl_d != -1) && pl_w != -1)
	{
		AQ_THROW( "Invalid Tenor String: Term must have format ON, TN, Y, M, W or D." )
	}

    // Check for valid strings
    // ==============================================================================
    
    // CHECK EVERY CHARACTER
    for( size_t i = 0; i < str.size(); ++i )
    {
        // Allow digits or Y, M, D, W characters or minus (negative terms) or blanks
        if ( !isdigit(str[i]) && str[i] != 'Y' && str[i] != 'M' && str[i] != 'W' && str[i] != 'D' && str[i] != '-' && str[i] != ' ' )
        {
            AQ_THROW( "Invalid Tenor String: Term must have format ON, TN, Y, M, W or D." )
        }
    }
    
    // ==============================================================================

	// Initialise return values
	y = 0;
	m = 0;
	d = 0;
	w = 0;

	// Insert into map in order to sort by position
	std::map<int, char> mp;
	if ( pl_y != -1 ) mp.insert(make_pair(pl_y, 'Y'));
	if ( pl_m != -1 ) mp.insert(make_pair(pl_m, 'M'));
	if ( pl_d != -1 ) mp.insert(make_pair(pl_d, 'D'));
	if ( pl_w != -1 ) mp.insert(make_pair(pl_w, 'W'));

	int currentPosition = 0;
	std::map<int, char>::iterator it = mp.begin();

	while ( it != mp.end() )
	{
		int endPosition = it->first;
		switch (it->second)
		{
			case 'Y':
				y = str.subString( currentPosition, endPosition ).getIntValue();
				break;

			case 'M':
				m = str.subString( currentPosition, endPosition ).getIntValue();
				break;

			case 'D':
				d = str.subString( currentPosition, endPosition ).getIntValue();
				break;

			case 'W':
				w = str.subString( currentPosition, endPosition ).getIntValue();
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
AQLMathDateCalculations::getDate(const AQLDate& basedate, 
                             const AQLString& term, 
                             bool rollForwards, 
                             const AQLString* roll_conv)
{
	int y, m, d, w;
	AQLMathDateCalculations::termStrtoYMDW(term, y, m, d, w);
	AQLDate date = basedate;
	if (!rollForwards)
	{
		y = -y;
		m = -m;
		d = -d;
		w = -w;
	}

	AQLString temp("");
	if (roll_conv) temp = *roll_conv;
    temp.toUpper();
	if (temp == ROLLCONV_LUNAR) // lunar case
	{
		date.addDays(y*13*28);
		date.addDays(m*28);
	}
	else
	{
		date.addYears(y);
		date.addMonths(m);
	}
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
	@param[in] rollForwards		true = roll forwards, false = roll backwards
	@param[in] roll_conv		roll convention
*/
AQLDate
AQLMathDateCalculations::getDate(const AQLDate& basedate, 
                                const AQLString& term, 
                                const AQLPriceDataSlidingRule& busdayrule,
                                const AQLPriceDataCalendar* pCal,
                                bool rollForwards,
							    const AQLString* roll_conv)
{
	int y, m, d,w;
	AQLMathDateCalculations::termStrtoYMDW(term, y, m, d, w);
	if (!rollForwards)
	{
		y = -y;
		m = -m;
		d = -d;
		w = -w;
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
	if (y != 0 || m != 0 || d == 0)
	{
        // Check if Lunar Roll Convention
        bool isLunarRollConvention = false;
        if( roll_conv != nullptr )
        {
            isLunarRollConvention = boost::iequals( (*roll_conv).getCString(), "LUNAR" );
        }

        // Special Case: Lunar Roll Convention
        if ( isLunarRollConvention )
        {
			date.addDays(y*13*28);
			date.addDays(m*28);
		}
        else
		{
			date.addYears(y);
			date.addMonths(m);
			if (roll_conv) date = rollDate(date, roll_conv);
		}
		date = slideDate(date, busdayrule, pCal);
	}

	// set D
	if (d == 0) return date;
	if (!roll_conv && busdayrule.getSlidingRule() != SLIDING_RULE_NO_CHANGE) // business day case
	{
		if (!pCal)
		{
            AQ_THROW("Invalid Date Calculation: Calendar is Required")
		}
		date = pCal->getBusinessDay(date, d);
	}
	else // actual day case
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
AQLMathDateCalculations::slideDate(const AQLDate& date, const AQLPriceDataSlidingRule& srule, const AQLPriceDataCalendar* pCal)
{
	if (srule.getSlidingRule() != SLIDING_RULE_NO_CHANGE) 
	{
		if (!pCal)
		{
			AQ_THROW("Invalid Date Calculation: Calendar is Required")
		}
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
AQLMathDateCalculations::rollDate(const AQLDate& basedate, const AQLString* roll)
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
AQLMathDateCalculations::getIMMDate(const int& y, const int& m, bool isOddMonth)
{
	if ((m%3 && !isOddMonth) || m <= 0 || m>=13) 
	{ 
		AQ_THROW("Invalid IMM month. The IMM month must be 3, 6, 9, or 12.")
	}

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
		AQ_THROW("Invalid Date: Invalid weekday provided")
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
AQLMathDateCalculations::getImmEndDate(const AQLDate& startDate, const AQLString& strTerm, int lag)
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
AQLMathDateCalculations::getFuturesContractStartDate(const unsigned int& month, const unsigned int& year )
{
	if ( month > 12 ) 
    {
        AQ_THROW( "Invalid Date: Invalid month provided" )
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
        AQ_THROW( "Invalid Date: Invalid weekday provided" )
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
AQLMathDateCalculations::getIMMDateFromTerm(const AQLDate& baseDate, const AQLString& futureTerm)
{
	AQ_THROW_IF(futureTerm.size() != 4, "Invalid Futures Contract Tenor: Futures contracts must be 4 letters" )
	
    AQ_THROW_IF(!isdigit(*futureTerm.subString(3, 3).getCString()), "Invalid Futures Contract Tenor: Futures contracts must end with a number" );
	
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
AQLMathDateCalculations::getFFDatesFromTerm(const AQLDate& baseDate, const AQLString& fedfundTerm)
{
	AQ_THROW_IF(fedfundTerm.size() != 4, "Invalid FED Fund Tenor Contract: FED Fund Futures contracts must be 4 letters");
	
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
AQLMathDateCalculations::getFXSpotDate(const AQLString& keyFX,
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
		AQLStringVector tempStrs(splitCalendarCentres(tempStr));
		tempStr = "";
		for(AQLStringVector::iterator it = tempStrs.begin(); it != tempStrs.end(); it++)
			if(*it != "NYB")
			{
				tempStr += *it;
				tempStr += CALENDAR_CENTRE_DELIMITER;
			}

		// Remove the trailing delimiter
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
AQLMathDateCalculations::haveNextCBDate(
	const AQLString& cb, const AQLDate& baseDate, bool strictlyAfter)
{
	AQLDate ignore;
	return getIfExistsNextCBDate(cb, baseDate, strictlyAfter, ignore);
}

/* static */ AQLDate
AQLMathDateCalculations::getNextCBDate(
	const AQLString& cb, const AQLDate& baseDate, bool strictlyAfter)
{
	AQLDate result;
	const bool exists = getIfExistsNextCBDate(cb, baseDate, strictlyAfter, result);

	if (!exists)
	{
		if (strictlyAfter)
		{
            AQ_THROW("No central bank data available after specified reference date");
		}
		else
		{
			AQ_THROW("No central bank data available on or after specified reference date");
		}
	}

	return result;
}

/* static */ bool
AQLMathDateCalculations::getIfExistsNextCBDate(
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
AQLMathDateCalculations::changeFutureMonthFormat(const AQLString& futureMonth)
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
        AQ_THROW("Invalid Futures Contract - Invalid futures month specified");
	}	
}


/*!
    @brief return convert from double grid to dates grid

	@param[in] asofDate
	@param[in] terms
	@param[out] dates
	
*/
void
AQLMathDateCalculations::convertToDateGrid(const AQLDate &asofDate, const DoubleArray &terms, DateVector &dates)
{
	dates.clear();
	
	const AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

	for (unsigned int i = 0; i < terms.size(); ++i)
	{
		const double term = terms[i];

		AQLDate date = AQLMathDateUtilities::getDateFromTerm( asofDate, term, dc_act365 );

		dates.push_back(date);
	}

	if (dates.size() != terms.size())
	{
        AQ_THROW("Inconsistent Data: Dates and Terms sizes must be the same.");
	}
}

// Return the frequency in months for comparing two frequencies, not for accurate calculations
// *** Duplicate method in AQLDateHelpers.cpp ***
double AQLMathDateCalculations::getPeriodFrequencyInMonths(const AQLString& freq)
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
AQLMathDateCalculations::calcCompoundingTimes(const AQLString& freq_rst, const AQLString& freq_pay)
{
	int resetFrequency = getPeriodFrequencyInMonths(freq_rst);
	int	paymentFrequency = getPeriodFrequencyInMonths(freq_pay); 

    if (paymentFrequency < resetFrequency)
	{
        AQ_THROW("Invalid Compound Frequency: Payment Frequency is more frequent than the reset frequency");
	}
	else return paymentFrequency / resetFrequency;
}

/* static */
AQLDate AQLMathDateCalculations::getNextWeekdayDate(
	AQLDayOfWeekEnum weekday, const AQLDate& baseDate, bool strictlyAfter)
{
	AQLDate res(baseDate);

	const unsigned int cur = static_cast<unsigned int>( baseDate.dayOfWeek() );
	assert(cur < 7);
	const unsigned int trg  = static_cast<unsigned int>( weekday );
	assert(trg < 7);
	const unsigned int diff = cur <= trg ? trg - cur : trg + 7 - cur;
	assert(0 <= diff && diff < 7);

	res.addDays(diff == 0 && strictlyAfter ? 7 : diff);

	assert(res.dayOfWeek() == weekday);

	return res;
}
