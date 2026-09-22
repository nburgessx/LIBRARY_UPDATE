/*! @file
    @brief Implementation to represent holiday information.
*/


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "AQLPriceDataCalendar.h"
#include "AQLDataVector.h"
#include "AQLCalendarSet.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLBasic.h"
#include "ExceptionMacros.h"
using namespace std;

AQLCalendar	AQLPriceDataCalendar::mStdCalendar;
bool		AQLPriceDataCalendar::mInitialize = false;

/*!
    @brief split a holiday-centre string on '+' (and, transitionally, ':')
*/
AQLStringVector splitCalendarCentres( const AQLString& centres )
{
	AQLString normalised = centres;
	normalised.exchange( ':', CALENDAR_CENTRE_DELIMITER );	// accept the legacy ':' during transition
	return normalised.toToken( CALENDAR_CENTRE_DELIMITER );
}

/*!
    @brief default constructor
*/
AQLPriceDataCalendar::AQLPriceDataCalendar(void)
: AQLPriceDataType(DATA_CALENDAR), mpCalendar(0) 
{
	if (!mInitialize)
	{
#ifndef MEMORY_CHECK
		mStdCalendar.setWeekly(SUN);
		mStdCalendar.setWeekly(SAT);
#endif
		mInitialize = true;
	}
	mCalendarSet.insert(&mStdCalendar);
	setNull(false);
}

/*!
    @brief constructor

    Process over the holiday to the city name.
	Information of city holiday city needs to be set to AQLCalendarSet object in advance.
	If you specify more than one city name, city information of each holiday is set in addition.

    @param[in] city a city holiday information to initialize(multi set is available with vector)
*/
AQLPriceDataCalendar::AQLPriceDataCalendar(AQLStringVector city)
	: AQLPriceDataType(DATA_CALENDAR), mpCalendar(0)
{
	AQLCalendarSet	calendarSet;

	int		i = -1;
	try
	{
		if (city.size() > 1)
		{
			//AQLCalendar		cal;
			for (i = 0; i < static_cast<int>(city.size()); i++)
			{
				//cal = calendarSet.getCalendar(city[i]);
				//mCalendar += cal;
				mCalendarSet.insert(&calendarSet.getCalendar(city[i]));
				mCity.insert(city[i]);
			}
		}
		else if (city.size() == 1)
		{
			i = 0;
			//mCalendar = calendarSet.getCalendar(city[i]);
			mCalendarSet.insert(&calendarSet.getCalendar(city[i]));
			mCity.insert(city[i]);
		}
		else
		{
			mCalendarSet.insert(&mStdCalendar);
		}
	}
	catch(AQLCoreError& e)
	{
		// exception
		AQLString	msg("InvalidData for AQLPriceDataCalendar : from ");
		if (i >= 0) {
			msg += city[i];
		}
		throw e += AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	setNull(false);
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLPriceDataCalendar::AQLPriceDataCalendar(const AQLPriceDataCalendar& attr)
				: AQLPriceDataType(DATA_CALENDAR), mCalendarSet(attr.mCalendarSet), 
				mCity(attr.mCity), mpCalendar(0)
{
	setNull(attr.isNull());
};

/*!
    @brief destructor
*/
AQLPriceDataCalendar::~AQLPriceDataCalendar(void) 
{
	if (mpCalendar)
	{
		delete mpCalendar;
	}
};

/*!
    @brief deep copy of the data object

    @return the copied object
*/
AQLPriceDataType*
AQLPriceDataCalendar::clone() const
{
    try {
    	AQLPriceDataCalendar*	pAttr = new AQLPriceDataCalendar(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief calendar city name in the string representation
    
    separated by a delimiter when use multi holiday cities (e.g. "NY:LDN:TKY")

    @return calendar city name separated by a delimiter
*/
AQLString
AQLPriceDataCalendar::convertToString(void) const
{
	AQLString	ret;
	if (isNull())
	{
		ret = NULL_STR;
	}
	else
	{
		ret = "";
		if (mCity.size())
		{
			set<AQLString>::const_iterator it = mCity.begin();
			set<AQLString>::const_iterator it_end = mCity.end();
			
			for (it_end--;it != it_end; ++it)
			{
				ret += *it;
				ret += CALENDAR_CENTRE_DELIMITER;
			}
			ret += *it;
		}
	}
	return ret;
}

/*!
    @brief get calendar that the class has
    
    @return calendar with holiday information
*/
const AQLCalendar &
AQLPriceDataCalendar::getCalendar(const bool isCache) const
{
	if (mpCalendar)
	{
		if (isCache)
		{
			return *mpCalendar;
		}
		else
		{
			delete mpCalendar;
			mpCalendar = new AQLCalendar;
		}
	}
	else
	{
		mpCalendar = new AQLCalendar;
	}
	set<const AQLCalendar *>::const_iterator it = mCalendarSet.begin();
	while (it != mCalendarSet.end())
	{
		(*mpCalendar) += *(*it);
		++it;
	}
	return *mpCalendar;
}

const set<const AQLCalendar *>&
AQLPriceDataCalendar::getCalendarSet(void)const
{
	return mCalendarSet;
}

/*!
    @brief get the business date added to (s) from base date(d)

    s is also available in a negative number

    @param[in] d base date
    @param[in] s business days to be added to the base date
    
    @return business day
*/
AQLDate				
AQLPriceDataCalendar::getBusinessDay(const AQLDate& d, int s) const
{
	AQLDate	ret = d;
	AQLDate	shift_pos;
	int		count = 0;
	int		next_step = 1;
	long num = AQLMath::abs(s);
	
	bool isHoliday = true;
	if (s == 0)
	{
		while (isHoliday)
		{
			isHoliday = false;
			set<const AQLCalendar *>::const_iterator it = mCalendarSet.begin();
			while (it != mCalendarSet.end())
			{
				if ((*it)->isHoliday(ret))
				{
					ret.addDays(next_step);
					isHoliday = true;
					break;
				}
				++it;
			}
		}
		return ret;
	}

	if (s < 0)
	{
		next_step = -1;
	}

	while (count < num)
	{
		isHoliday = true;
		while (isHoliday)
		{
			isHoliday = false;
			ret.addDays(next_step);
			count++;
			set<const AQLCalendar *>::const_iterator it = mCalendarSet.begin();
			while (it != mCalendarSet.end())
			{
				if ((*it)->isHoliday(ret))
				{
					isHoliday = true;
					count--;
					break;
				}
				++it;
			}
		}
	}

	return ret;

	//if (s == 0)
	//{
	//	while (mCalendar.isHoliday(ret))
	//	{
	//		ret.addDays(next_step);
	//	}
	//	return ret;
	//}

	//if (s < 0)
	//{
	//	next_step = -1;		// shift amount (when negative)
	//}
	//
	//ret.addDays(s);

	//count = mCalendar.countHoliday(d, ret);  // get the number of days of holiday for the period
	//
	//if (mCalendar.isHoliday(d))
	//{
	//	--count;
	//}
	//if (count < 2)
	//{
	//	while( count)
	//	{
	//		ret.addDays(next_step);
	//		if (!mCalendar.isHoliday(ret))
	//		{
	//			return ret;
	//		}
	//	}
	//}
	//else
	//{
	//	while (count)
	//	{
	//		shift_pos = ret;
	//		shift_pos.addDays(next_step);
	//		ret.addDays(count * next_step);
	//		count = mCalendar.countHoliday(shift_pos, ret);
	//	}
	//}
	//return ret;
}

/*!
    @brief get the number of business days between two dates

    @param[in] fromDate
    @param[in] toDate
    
    @return number of business days between the from and to date
*/
int
AQLPriceDataCalendar::businessDays( const AQLDate& fromDate, const AQLDate& toDate ) const
{
    int nBusinessDays = 0;
    AQLDate tempDate = fromDate;
    do 
    {
        tempDate = AQLPriceDataCalendar::getBusinessDay( fromDate, nBusinessDays );
        nBusinessDays = nBusinessDays + 1;
    } while ( tempDate < toDate );

    nBusinessDays = nBusinessDays - 1;
    return nBusinessDays;
}


/*!
    @brief return EOM date

    @param[in] d 
     
    @return EOM date
*/
AQLDate				
AQLPriceDataCalendar::getEOMDay(const AQLDate& d) const
{
	AQLDate eomDate = d;
	eomDate.addDays(d.intervalToEndOfMonth());
	bool isHoliday = true;
	while (isHoliday)
	{
		isHoliday = false;
		set<const AQLCalendar *>::const_iterator it = mCalendarSet.begin();
		while (it != mCalendarSet.end())
		{
			if ((*it)->isHoliday(eomDate))
			{
				isHoliday = true;
				eomDate.addDays(-1);
				break;
			}
			++it;
		}
	}

	//while (mCalendar.isHoliday(eomDate))
	//{
	//	eomDate.addDays(-1);
	//}
	return eomDate;
}

/*!
    @brief return EOW(end of week) date

    @param[in] d 
     
    @return EOW date
*/
AQLDate				
AQLPriceDataCalendar::getEOWDay(const AQLDate& d) const
{
	AQLDate eowDate = d;
	AQLDayOfWeekEnum w = d.dayOfWeek();
	eowDate.addDays(FRI - w); //set to Friday
	bool isHoliday = true;
	while (isHoliday)
	{
		isHoliday = false;
		set<const AQLCalendar *>::const_iterator it = mCalendarSet.begin();
		while (it != mCalendarSet.end())
		{
			if ((*it)->isHoliday(eowDate))
			{
				isHoliday = true;
				eowDate.addDays(-1);
				break;
			}
			++it;
		}
	}
	return eowDate;
}

/*!
	@brief return BOM (beginning of month) date

	@param[in] d AQLDate input date

	@return first business day of the month within which the input date falls
*/
AQLDate
AQLPriceDataCalendar::getBOMDay(const AQLDate& d) const
{
	AQLDate bomDate = d;
	bomDate.addDays(d.intervalToStartOfMonth());
	bool isHoliday = true;
	while (isHoliday)
	{
		isHoliday = false;
		set<const AQLCalendar *>::const_iterator it = mCalendarSet.begin();
		while (it != mCalendarSet.end())
		{
			if ((*it)->isHoliday(bomDate))
			{
				isHoliday = true;
				bomDate.addDays(+1);
				break;
			}
			++it;
		}
	}
	return bomDate;
}

/*!
    @brief set the calendar holiday information from a string of city names that you specify

    separated by a delimiter when use multi holiday cities (e.g. "NY:LDN:TKY")

    @param[in] str string representing city name
*/
void
AQLPriceDataCalendar::convertFromString(const AQLString& str)
{
	
	AQLCalendarSet		calendarSet;
	mCity.clear();
	mCalendarSet.clear();
	setNull();
		
	AQLString data;
	bool ret = strToData(str, data);
	if (ret || data.size() == 0) 
	{
		//mCalendar = mStdCalendar;
		mCalendarSet.insert(&mStdCalendar);
		// notify of the change
		update();
		return;
	}
	
	if (data == "" || data == NULL_STR)
	{
		//mCalendar = mStdCalendar;
		mCalendarSet.insert(&mStdCalendar);
		// notify of the change
		update();
		return;
	}

	setNull(false);
	vector<AQLString> tokens = splitCalendarCentres( data );
	vector<AQLString>::iterator it = tokens.end();
	try
	{
		if (tokens.size() > 1)
		{
			for (it = tokens.begin();it != tokens.end(); ++it)
			{
				//AQLCalendar	cal;
				//cal = calendarSet.getCalendar(*it);  // get the calendar from the city name
				//mCalendar += cal;
				mCalendarSet.insert(&calendarSet.getCalendar(*it));
				mCity.insert(*it);
			}
		}
		else if (tokens.size() == 1)
		{
			it = tokens.begin();
			//mCalendar = calendarSet.getCalendar(*it);
			mCalendarSet.insert(&calendarSet.getCalendar(*it));
			mCity.insert(*it);
		}
		else
		{
			mCalendarSet.insert(&mStdCalendar);
			//mCalendar = mStdCalendar;
		}
	}
	catch (AQLCoreError& e)
	{
		setNull(true);
		AQ_THROW( "Invalid Holiday Calendar(s): " + str )
	}
	
	// notify of the change
	update();
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
AQLPriceDataType&
AQLPriceDataCalendar::assignment(const AQLPriceDataType& a)
{
	if (this == &a) return *this;
	if (a.getType() != DATA_CALENDAR)
	{
		// exception
		AQLString	msg("Assignment error for AQLPriceDataCalendar : from ");
		msg += AQLString(a.getType());
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	update();
	//mCalendar = dynamic_cast<const AQLPriceDataCalendar&>(a).mCalendar;
	mCalendarSet = dynamic_cast<const AQLPriceDataCalendar&>(a).mCalendarSet;
	setNull(a.isNull());
	return *this;
}

/*
    @brief compare the contents against the other objecet (Magnitude comparison is carried out by the number of city holiday calendar information.)
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
    
    @return (number of city names that this Data has) - (number of city names that Data "a" has)
*/
int
AQLPriceDataCalendar::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_CALENDAR)
	{
		// exception
		AQLString	msg("Compare error for AQLPriceDataCalendar : from ");
		msg += AQLString(a.getType());
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	if (isNull() && a.isNull())
	{
		return 0;
	}
	if (isNull())
	{
		return -1;
	}
	if (a.isNull())
	{
		return 1;
	}
	return mCity.size() - dynamic_cast<const AQLPriceDataCalendar&>(a).mCity.size(); 
}
