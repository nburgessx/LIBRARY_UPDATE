// LAPriceDataDayCount.cpp

#ifdef __GNUG__
#pragma implementation
#endif

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "LABasic.h"
#include "LAPriceDataDayCount.h"
#include "LADate.h"
#include "LACoreAppError.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include "ExceptionMacros.h"	// include: MLIB_REQUIRE, MLIB_THROW


// Function to round a double to the nearest integer
int roundToNearest(double number)
{
    // Use floor( d + 0.5 ) for positive doubles and ceil( d - 0.5 ) for negative doubles
    return number < 0.0 ? int( ceil(number - 0.5) ) : int( floor(number + 0.5) );
}

using namespace std;

// DayCount-rule SIZE
#define DAY_C_SIZE 11

// DayCount-rule Strings
static const char_t* gDayCount[DAY_C_SIZE] =
{
    "ACT/ACT", "ACT/365", "ACT/360", "30/360", "30E/360", "ACT/365_ISDA" ,"ACT/365FJ", "1/1", "1/2", "1/4", "ACT/ACT.ICMA"
};

/*!
    @brief default_constructor

    Actual/365(ISDA RULE) is default
*/
LAPriceDataDayCount::LAPriceDataDayCount(const staticData_t attr) : LAPriceDataType(attr)
{
    mDayCount = ACT_ACT;
}

/*!
    @brief copy_constructor

    @param[in] attr Orginal_Object
*/
LAPriceDataDayCount::LAPriceDataDayCount(const LAPriceDataDayCount& attr) 
    : LAPriceDataType(attr)
{
    mDayCount = attr.mDayCount;
}

/*!
    @brief constructor

    @param[in] dc DayCount rule

*/
LAPriceDataDayCount::LAPriceDataDayCount(const DayCount dc)

    : LAPriceDataType(DATA_DAYCOUNT)
{
    mDayCount = dc;
    setNull(false);
}

/*!
    @brief destructor
*/
LAPriceDataDayCount::~LAPriceDataDayCount(void)
{
}

/*!
    @brief Copy Object of this Data

    @return Copy Object
*/
LAPriceDataType*    
LAPriceDataDayCount::clone() const
{
    try {
        LAPriceDataDayCount*   pAttr = new LAPriceDataDayCount(*this);
        return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief Return DayCount-rule by LAString
           ex.) "ACT_365_ISDA"

    @return DayCount-rule by LAString

    @sa enum DayCount
*/
LAString   
LAPriceDataDayCount::convertToString(void) const
{
    if (isNull()) return NULL_STR;
    LAString ret(gDayCount[mDayCount]);
    return ret;
}

/*!
    @brief Return Term between fromDate to toDate by year-count
    
    Culc "End_Date - Start_Date" (If End_Date = Start_Date, Return 0)

    @param[in] fromDate		Start_Date
    @param[in] toDate		End_Date 
	@param[in] includelast  true(defalt):include last day and not include start day,
							false		:include start day and not include last day 

    @return Term between fromDate to toDate by year-count
*/
double
LAPriceDataDayCount::getTerm(const LADate& fromDate, const LADate& toDate, bool includelast) const
{
    if (toDate < fromDate) return -getTerm(toDate, fromDate);
	if (fromDate == toDate) return 0;
	


	double ret = 0;
    switch (mDayCount)
    {
    // Most of the time we will be using ACT/365, so declare this case first
    case ACT_365:
        ret = fromDate.intervalDays(toDate) / 365.0;
        break;
    case ACT_360:
        ret = fromDate.intervalDays(toDate) / 360.0;
        break;
    case N30_360:
        { 
			int d1 = (fromDate.dayOfMonth() == 31) ? 30 : fromDate.dayOfMonth();
			int d2 = ((toDate.dayOfMonth() == 31) && (d1 == 30)) ? 30 : toDate.dayOfMonth();

			ret = ( (toDate.yearOfEra() - fromDate.yearOfEra()) * 360
               + (toDate.monthOfYear() - fromDate.monthOfYear()) * 30
               + (d2 - d1) ) / 360.0;
        }
        break;
	case E30_360:
		{
			int d1 = (fromDate.dayOfMonth() == 31) ? 30 : fromDate.dayOfMonth();
			int d2 = (toDate.dayOfMonth() == 31) ? 30 : toDate.dayOfMonth();
				
			ret = ( (toDate.yearOfEra() - fromDate.yearOfEra()) * 360
               + (toDate.monthOfYear() - fromDate.monthOfYear()) * 30
               + (d2 - d1) ) / 360.0;
		}
		break;
	case ACT_ACT:
	case ACT_365_ISDA:
		{	
			// TODO: remove this commented out code once checked
			int y1 = fromDate.yearOfEra();
			int y2 = toDate.yearOfEra();

			int td1 = (includelast) ? fromDate.intervalToEndOfYear() : fromDate.intervalToEndOfYear() + 1;
			int ty1 = 365 + fromDate.isLeapYear();
			int ty2 = 365 + toDate.isLeapYear();
			int td2 = (includelast) ? ty2 - toDate.intervalToEndOfYear() : ty2 - toDate.intervalToEndOfYear() - 1;

			ret = (y2 - y1 - 1) + (ty2 * td1 + ty1 * td2) / static_cast<double>(ty1 * ty2);			
			return ret;
			
			/*
			// https://en.wikipedia.org/wiki/Day_count_convention#Actual.2FActual_ISDA

			// const boost::gregorian::date asOfDate(effectiveDate.yearOfEra(),effectiveDate.monthOfYear(), effectiveDate.dayOfMonth());
			auto boostFromDate = boost::gregorian::from_undelimited_string(fromDate.stringWithFormat().getCString());
			auto boostToDate = boost::gregorian::from_undelimited_string(toDate.stringWithFormat().getCString());
            double includeAdjusment = includelast ? 1.0 : 0.0;

			int y1 = boostFromDate.year();
            int y2 = boostToDate.year();
            if( y2 == y1 )
            {
                double denominator = fromDate.isLeapYear() ? 366.0 : 365.0;
                return ( boost::gregorian::date_period( boostFromDate, boostToDate ).length().days() + includeAdjusment ) / denominator;
            }

            auto daysFromDateToEOY	= boost::gregorian::date_period( boostFromDate, boost::gregorian::date( boostFromDate.year() + 1, 1, 1 ) ).length().days();
			auto daysStartOfYearToToDate = boost::gregorian::date_period( boost::gregorian::date( boostToDate.year(), 1, 1 ), boostToDate ).length().days() + includeAdjusment;

            int daysInLeapYear = ( fromDate.isLeapYear() ? daysFromDateToEOY - 1 : 0 ) + ( toDate.isLeapYear() ? daysStartOfYearToToDate: 0 );
            int daysNotInLeapYear = ( !fromDate.isLeapYear()  ? daysFromDateToEOY - 1 : 0 ) + ( !toDate.isLeapYear() ? daysStartOfYearToToDate : 0 );

            int yearCounter = y1 + 1;
            while( yearCounter < y2 )
            {
                if( IS_LEAP_YEAR( yearCounter ) )
                {
                    daysInLeapYear += 366.0;
                }
                else
                {
                    daysNotInLeapYear += 365.0;
                }
                yearCounter++;
            }
            return ( daysInLeapYear / 366.0 + daysNotInLeapYear / 365.0 );		 
			*/
		}
		break;
    case ACT_365_FJ:
		{
			int y1 = fromDate.monthOfYear();
			int y2 = toDate.monthOfYear();
			int d1 = fromDate.intervalYears(toDate) ;
			LADate tmpDate = fromDate;
			int f = 0;
			int i;
			
			if (y1 < 3 && fromDate.isLeapYear() ) f++; //add followed condition : && fromDate.stringWithFormat("MMDD") != "0229"
			if (y2 > 2 && toDate.isLeapYear() && fromDate.yearOfEra() != toDate.yearOfEra()) f++;
			if (includelast  && y1 == 2 && fromDate.dayOfMonth() == 29) f--;
			if (!includelast && y2 == 2 && toDate.dayOfMonth()   == 29) f--;

			for (i = 0; i < d1 - 1; i++)
				{
					tmpDate.addYears(1);
					if (tmpDate.isLeapYear()) f++;
			    }
			ret = (fromDate.intervalDays(toDate) - f) / 365.0;
		}
		break;
	case DAYCOUNT_ONE:
		{
			ret = 1;
		}
		break;
	case DAYCOUNT_HALF:
		{
			ret = 0.5;
		}
		break;
	case DAYCOUNT_QUARTER:
		{
			ret = 0.25;
		}
		break;
	case ACT_ACT_ICMA:
		{
			// merge StartDates and EndDates as a single date vector
			std::vector<LADate> CouponStartEndDates;
			std::set_union(mCouponStartDates.begin(), mCouponStartDates.end(), 
						   mCouponEndDates.begin(), mCouponEndDates.end(),
						   std::inserter(CouponStartEndDates, CouponStartEndDates.end()));

			int i1 = -1;
			int i2 = -1;
			for(unsigned int i = 0; i < CouponStartEndDates.size(); ++i)
			{
				if(CouponStartEndDates[i] > fromDate)
				{
					i1 = i;
					break;
				}
			}
			for(unsigned int i = 0; i < CouponStartEndDates.size(); ++i)
			{
				if(CouponStartEndDates[i] >= toDate)
				{
					i2 = i;
					break;
				}
			}

			if(i1 < 0 || i2 < 0)
			{
				LAString msg = "Daycount ACT/ACT.ICMA: FromDate or ToDate are out of range.";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			int DaysAccrued, DaysInTheYear;
			if(i1 == i2)
			{
				DaysAccrued = fromDate.intervalDays(toDate);
				DaysInTheYear = CouponStartEndDates[i1 - 1].intervalDays(CouponStartEndDates[i1]) * mCouponsPerYear;
				ret += (double)(DaysAccrued)/(double)(DaysInTheYear);
			}
			else
			{
				DaysAccrued = fromDate.intervalDays(CouponStartEndDates[i1]);
				DaysInTheYear = CouponStartEndDates[i1 - 1].intervalDays(CouponStartEndDates[i1]) * mCouponsPerYear;
				ret += (double)(DaysAccrued)/(double)(DaysInTheYear);

				for(unsigned int i = 0; i < i2 - i1 -1; ++i)
				{
					ret += 1.0/(double)(mCouponsPerYear);
				}

				DaysAccrued = CouponStartEndDates[i2 - 1].intervalDays(toDate);
				DaysInTheYear = CouponStartEndDates[i2 - 1].intervalDays(CouponStartEndDates[i2]) * mCouponsPerYear;
				ret += (double)(DaysAccrued)/(double)(DaysInTheYear);
			}
		}
		break;

	}
    return ret;
}
/*!
    @brief Return Term from fromDate by year-count
    
    @param[in] fromDate Start_Date
    @param[in] termDays Term by day-count 
	@param[in] includelast  true(defalt):include last day and not include start day,
							false		:include start day and not include last day 


    @return Term from fromDate by year-count
*/

double
LAPriceDataDayCount::getTerm(const LADate& fromDate, const double& termDays, bool includelast) const
{
    double ret = 0;
    switch (mDayCount)
    {
    case ACT_360:
        ret = termDays / 360.0;
        break;
   // case ACT_ACT:
   //     {
			//if (!includelast)
			//{
			//	LADate from = fromDate;
			//	from.addDays(-1);
			//	return getTerm(from, termDays, true);
			//}	            
   //         
			//
			//LADate tmpDate(fromDate);
   //         int days = static_cast<int>(termDays);
   //         tmpDate.addDays(days);
   //         int years = fromDate.intervalYears(tmpDate);
   //         tmpDate = fromDate;
   //         tmpDate.addYears(years);
   //         LADate tmpDate2(tmpDate);
   //         tmpDate2.addYears(1);       
   //         ret = (double)years + (termDays - fromDate.intervalDays(tmpDate))/((double)tmpDate.intervalDays(tmpDate2));
   //     }
   //     break;
	case ACT_365:
		ret = termDays / 365.0;
        break;
    case N30_360:
	case E30_360:
		{
			int days = static_cast<int>(ceil(termDays));
            double points = days - termDays;
            LADate toDate(fromDate);
            toDate.addDays(days);  
            
            ret = getTerm(fromDate, toDate) - points / 360.0;
		}
		break;
	case ACT_ACT:
	case ACT_365_ISDA:
		{
            int days = static_cast<int>(ceil(termDays));
			LADate tmpDate = fromDate;
			tmpDate.addDays(days);

			int y1 = fromDate.yearOfEra();
			int y2 = tmpDate.yearOfEra();

			int td1 = (includelast) ? fromDate.intervalToEndOfYear() : fromDate.intervalToEndOfYear() + 1;
			int ty1 = 365 + fromDate.isLeapYear();
			int ty2 = 365 + tmpDate.isLeapYear();
			double td2 = (includelast) ? ty2 - tmpDate.intervalToEndOfYear() - (days - termDays)
				: ty2 - tmpDate.intervalToEndOfYear() - (days - termDays) - 1;

			ret = (y2 - y1 - 1) + (ty2 * td1 + ty1 * td2) / static_cast<double>(ty1 * ty2);			
		}
		break;
	case ACT_365_FJ:
		{
			int days = static_cast<int>(ceil(termDays));
			LADate toDate = fromDate;
			toDate.addDays(days);

			int y1 = fromDate.monthOfYear();
			int y2 = toDate.monthOfYear();
			int d1 = fromDate.intervalYears(toDate) ;
			LADate tmpDate = fromDate;
			int f = 0;
			if (y1 < 3 && fromDate.isLeapYear()) f++;
			if (y2 > 2 && toDate.isLeapYear() && d1 != 0) f++;
			if (includelast  && y1 == 2 && fromDate.dayOfMonth() == 29) f--;
			if (!includelast && y2 == 2 && toDate.dayOfMonth()   == 29) f--;

			for (int i = 0; i < d1 - 1; i++)
				{
					tmpDate.addYears(1);
					if (tmpDate.isLeapYear()) f++;
			    }
			ret = (fromDate.intervalDays(toDate) - f) / 365.0;
		}
		break;
	case DAYCOUNT_ONE:
		{
			ret = 1;
		}
		break;
	case DAYCOUNT_HALF:
		{
			ret = 0.5;
		}
		break;
	case DAYCOUNT_QUARTER:
		{
			ret = 0.25;
		}
		break;
	case ACT_ACT_ICMA:
		{
			throw LACoreInvalidData("GetTerm(FromDate, TermDays) is not supported for daycount ACT/ACT.ICMA.", __FILE__, __LINE__);
		}
		break;

	}
    return ret;
}
/*!
    @brief Returns the Numerator for the Daycount used 

    @param[in] fromDate		Start_Date
    @param[in] toDate		End_Date 
	@param[in] includelast  true(defalt):include last day and not include start day,
							false		:include start day and not include last day 

    @return number of days between fromDate to toDate by year-count
*/

int
LAPriceDataDayCount::getNumerator(const LADate& fromDate, const LADate& toDate, bool includelast) const
{
    if (toDate < fromDate) return -getTerm(toDate, fromDate);
	if (fromDate == toDate) return 0;
	


	int ret = 0;
    switch (mDayCount)
    {
    case ACT_360:
        ret = fromDate.intervalDays(toDate);
        break;
	case ACT_365:
        ret = fromDate.intervalDays(toDate);
        break;
    case N30_360:
        { 
			int d1 = (fromDate.dayOfMonth() == 31) ? 30 : fromDate.dayOfMonth();
			int d2 = ((toDate.dayOfMonth() == 31) && (d1 == 30)) ? 30 : toDate.dayOfMonth();

			ret = ( (toDate.yearOfEra() - fromDate.yearOfEra()) * 360
               + (toDate.monthOfYear() - fromDate.monthOfYear()) * 30
               + (d2 - d1) );
        }
        break;
	case E30_360:
		{
			int d1 = (fromDate.dayOfMonth() == 31) ? 30 : fromDate.dayOfMonth();
			int d2 = (toDate.dayOfMonth() == 31) ? 30 : toDate.dayOfMonth();
				
			ret = ( (toDate.yearOfEra() - fromDate.yearOfEra()) * 360
               + (toDate.monthOfYear() - fromDate.monthOfYear()) * 30
               + (d2 - d1) );
		}
		break;
	case ACT_ACT:
	case ACT_365_ISDA:
		{
	
			throw LACoreInvalidData(" ACT_ACT and ACT_365_ISDA are not supported for this method", __FILE__, __LINE__);	
		 
		}
		break;
    case ACT_365_FJ:
		{
			int y1 = fromDate.monthOfYear();
			int y2 = toDate.monthOfYear();
			int d1 = fromDate.intervalYears(toDate) ;
			LADate tmpDate = fromDate;
			int f = 0;
			int i;
			
			if (y1 < 3 && fromDate.isLeapYear() ) f++; //add followed condition : && fromDate.stringWithFormat("MMDD") != "0229"
			if (y2 > 2 && toDate.isLeapYear() && fromDate.yearOfEra() != toDate.yearOfEra()) f++;
			if (includelast  && y1 == 2 && fromDate.dayOfMonth() == 29) f--;
			if (!includelast && y2 == 2 && toDate.dayOfMonth()   == 29) f--;

			for (i = 0; i < d1 - 1; i++)
				{
					tmpDate.addYears(1);
					if (tmpDate.isLeapYear()) f++;
			    }
			ret = (fromDate.intervalDays(toDate) - f);
		}
		break;
	case DAYCOUNT_ONE:
	case DAYCOUNT_HALF:
	case DAYCOUNT_QUARTER:
	case ACT_ACT_ICMA:
		{
			throw LACoreInvalidData(" This daycount is not supported for this method", __FILE__, __LINE__);	
		}
		break;

	}
    return ret;
}
/*!
    @brief Returns the Denominator for the Daycount used 
    @return number of days in a year for the Daycount used
*/

int
LAPriceDataDayCount::getDenominator() const
{
	
	int ret = 0;
    switch (mDayCount)
    {
    case ACT_360:
        ret = 360;
        break;

	case ACT_365:
        ret = 365;
        break;
    case N30_360: 
		ret = 360;
        break;
	case E30_360:
		ret = 360;
		break;
	case ACT_ACT:
	case ACT_365_ISDA:
		{
			throw LACoreInvalidData(" ACT_ACT and ACT_365_ISDA are not supported for this method", __FILE__, __LINE__);
		}
		break;
    case ACT_365_FJ:
		{
			ret = 365;
		}
		break;
	case DAYCOUNT_ONE:
	case DAYCOUNT_HALF:
	case DAYCOUNT_QUARTER:
	case ACT_ACT_ICMA:
		{
			throw LACoreInvalidData(" This daycount is not supported for this method", __FILE__, __LINE__);	
		}
		break;
	}
    return ret;
}

// Static Method to get etrading DayCountEnum
etrading::DayCountEnum
LAPriceDataDayCount::dayCountEnum( const DayCount & daycount )
{
	etrading::DayCountEnum result = etrading::NONE_DAYCOUNT;
    
	switch ( daycount )
    {
		case ACT_365:
		{
			// Actual/365(Fixed)
			result = etrading::ACT_365_DAYCOUNT;
			break;
		}
		case ACT_360:
		{
			// Actual/360
			result = etrading::ACT_360_DAYCOUNT;
			break;
		}
		case ACT_ACT:
		{
			// Actual/Actual
			result = etrading::ACT_ACT_DAYCOUNT;
			break;
		}
		case N30_360:
		{
			// 30/360
			result = etrading::N30_360_DAYCOUNT;
			break;
		}
		case E30_360:
		{
			// 30E/360
			result = etrading::E30_360_DAYCOUNT;
			break;
		}
		case ACT_ACT_ICMA:
		{
			// Actual/Actual(ICMA RULE)
			// TODO: *** Fix this **** We use ACT/ACT as a proxy, since this does not appear to have been implemented
			result = etrading::ACT_ACT_DAYCOUNT;
			break;
		}
		case ACT_365_ISDA:
		{
			// Actual/365(ISDA RULE)
			// Note: ACT/ACT has been implemented the same as ACT/365_ISDA
			result = etrading::ACT_ACT_DAYCOUNT;
			break;
		}
		case ACT_365_FJ:
		{
			// Actual/365FJ
			result = etrading::ACT_365_FJ_DAYCOUNT;
			break;
		}
		case DAYCOUNT_ONE:
		{
			// 1/1
			result = etrading::ONE_DAYCOUNT;
			break;
		}
		case DAYCOUNT_HALF:
		{
			// 1/2
			result = etrading::HALF_DAYCOUNT;
			break;
		}
		case DAYCOUNT_QUARTER:
		{
			// 1/4
			result = etrading::QUARTER_DAYCOUNT;
			break;
		}
		default:
		{
			MLIB_THROW("Invalid Daycount: Daycount not supported in LAPriceDataDayCount class")
			break;
		}
	}
    return result;
}

// Get Etrading DayCountEnum
etrading::DayCountEnum LAPriceDataDayCount::dayCountEnum() const
{
	etrading::DayCountEnum result = LAPriceDataDayCount::dayCountEnum( mDayCount );
	return result;
}

/*!
    @brief Return Term from fromDate by day-count

    @param[in] fromDate Start_Date
    @param[in] termY	Term by year-count 
	@param[in] includelast  true(defalt):include last day and not include start day,
							false		:include start day and not include last day 


    @return Term from fromDate by day-count
*/

double
LAPriceDataDayCount::getDayTerm(const LADate& fromDate, const double& termY, bool includelast) const
{
    // Important Bug Fix: 20-Jul-16
    // If a year fraction has been generated from a start and end date and a daycount then we should get
    // an exact julius integer result when returning to the same end date when performing the reverse operation.
    // Precision issues can result in incorrect dates being generated when we cast the result of this function
    // to int, in some cases an int of 1.9999999 is cast to 1 giving the wrong date. So we round results to the
    // nearest int to avoid this issue.
    
    double ret = 0;
    switch (mDayCount)
    {
    case ACT_360:
        ret = roundToNearest( termY * 360.0 );
        break;
   // case ACT_ACT:
   //     {
			//if (!includelast)
			//{
			//	LADate from = fromDate;
			//	from.addDays(-1);
			//	return getDayTerm(from, termY, true);
			//}

			//LADate tmpDate(fromDate);
   //         int year = static_cast<int>(termY);
   //         tmpDate.addYears(year);
   //         LADate tmpDate2(tmpDate);
   //         tmpDate2.addYears(1);

   //         ret = fromDate.intervalDays(tmpDate) + (termY - (double)year)*tmpDate.intervalDays(tmpDate2);
   //     }
   //     break;
	case ACT_365:
        ret = roundToNearest( termY * 365.0 );
        break;
	case N30_360:
	case E30_360:	case ACT_365_FJ:
		{
			throw LACoreInvalidData("Daycounts: N30_360, E30_360 and ACT_365_FJ are not supported in this method", __FILE__, __LINE__);
		}
		break;
	case ACT_ACT:
	case ACT_365_ISDA:
		{
			double td1 = (includelast) ? fromDate.intervalToEndOfYear() : fromDate.intervalToEndOfYear() + 1;
			double ty1 = static_cast<double>(365 + fromDate.isLeapYear());
		
			ret = 0.0;
			if (td1 / ty1 >= termY) ret = termY * ty1;
			else
			{
				LADate tmpDate = fromDate;
				tmpDate.addDays(fromDate.intervalToEndOfYear());
				double term = termY - td1 / ty1;
				int year = static_cast<int>(term);
				tmpDate.addYears(year);
				term -= year;

				tmpDate.addDays(1);
				int ty2 = 365 + tmpDate.isLeapYear();
				int days = static_cast<int>(term * ty2);
				tmpDate.addDays(days - 1);
				ret = fromDate.intervalDays(tmpDate) + roundToNearest( term * ty2 - days );
				if (!includelast) ret += 1;
			}
		
		}
		break;
	case DAYCOUNT_ONE:
		{
			LAString msg;
			msg += "This function is not supported for DAYCOUNT_ONE";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		break;
	case DAYCOUNT_HALF:
		{
			LAString msg;
			msg += "This function is not supported for DAYCOUNT_HALF";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		break;
	case DAYCOUNT_QUARTER:
		{
			LAString msg;
			msg += "This function is not supported for DAYCOUNT_QUARTER";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		break;
	case ACT_ACT_ICMA:
		{
			throw LACoreInvalidData("GetDayTerm(FromDate, TermYear) is not supported for daycount ACT/ACT.ICMA.", __FILE__, __LINE__);
		}
		break;
    }
    return ret;
}

/*!
    @brief Set Data of DayCount by String of DayCount-rule
   
    @param[in] str String of DayCount-rule (e.g. "CONT(ACT_365_ISDA)")

    @sa enum RateConvention, enum DayCount
*/
void          
LAPriceDataDayCount::convertFromString(const LAString& str)
{
    LAString data;
    
    bool ret = strToData(str, data);
    if (! ret && data.size() != 0)
    {
        int i;
        // Set DAY COUNT BASIS
        for (i = 0; i < DAY_C_SIZE; i++)
        {
            if (data == gDayCount[i])
            {
				if (mDayCount !=  DayCount(i))
				{
					mDayCount = DayCount(i);
					update();
				}
                break;
            }
        }
        if (i == DAY_C_SIZE)
        {
            LAString msg("Invalid String for convertFromString : ");
            msg += str;
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }
        setNull(false);
    }
	else
	{
		// Set DEFAULT
		mDayCount = ACT_365_ISDA;
		update();   // Update
		setNull();  // Set NULL
	}
}

/*!
    @brief Set DayCount

    @param[in] dc DayCount
*/
void                  
LAPriceDataDayCount::setDayCount(const DayCount dc)
{
	if (mDayCount != dc)
	{
		mDayCount = dc;
		setNull(false);
		// Update
		update();
	}
}

/*!
    @brief Set DayCount

    @param[in] dc DayCount by LAString
*/
void                  
LAPriceDataDayCount::setDayCount(const LAString& dc)
{
    int i;
    for (i = 0; i < DAY_C_SIZE; i++)
    {
        if (dc == gDayCount[i])
        {
			if (mDayCount != DayCount(i))
			{
				mDayCount = DayCount(i);
				// Update
				update();

			}
            break;
        }
    }
    if (i == DAY_C_SIZE)
    {
        LAString msg("Invalid String for setDayCount : ");
        msg += dc;
        throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    setNull(false);
}
/*!
    @brief initialize this class with another

	@param[in] a object to be referenced for initialization

	@return initialized object(this object)
*/

LAPriceDataType& 
LAPriceDataDayCount::assignment(const LAPriceDataType& a)
{
    if (this == &a) return *this;

    if (a.getType() != DATA_DAYCOUNT) 
    {
        LAString err = "Assignement error for LAPriceDataDayCount : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    mDayCount = dynamic_cast<const LAPriceDataDayCount&>(a).mDayCount;
    
    setNull(a.isNull());
    // Update
    update();
    return *this;
}
/*!
    @brief compare this object with another 
	if argument is not LAPriceDataDayCount, then error
    @param[in] a object to be compared

    @return 1 :equal, 0 :not-equal
*/

int          
LAPriceDataDayCount::compare(const LAPriceDataType& a) const
{
    if (a.getType() != DATA_DAYCOUNT) 
    {
        LAString err = "Compare error for LAPriceDataDayCount : from ";
        err += LAString(a.getType());
        throw LACoreInvalidData(err.getCString(), __FILE__, __LINE__);
    }
    const LAPriceDataDayCount& at = dynamic_cast<const LAPriceDataDayCount&>(a);
   
    int ret = (at.mDayCount - mDayCount);
    return ret;
}
