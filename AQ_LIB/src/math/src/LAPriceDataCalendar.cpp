/*! @file
    @brief Implementation to represent holiday information.

*/


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "LAPriceDataCalendar.h"
#include "LADataVector.h"
#include "LAMathCalendarSet.h"
#include "LAPriceDataSlidingRule.h"
#include "LABasic.h"
#ifdef __HAS_MIC__

#endif
using namespace std;

#ifdef __HAS_MIC__
common_lib::StaticMutex LAPriceDataCalendar::mMutex;
#endif
LAMathCalendar	LAPriceDataCalendar::mStdCalendar;
bool		LAPriceDataCalendar::mInitialize = false;

/*!
    @brief default constructor
*/
LAPriceDataCalendar::LAPriceDataCalendar(void)
: LAPriceDataType(DATA_CALENDAR), mpCalendar(0) 
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
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
	Information of city holiday city needs to be set to LAMathCalendarSet object in advance.
	If you specify more than one city name, city information of each holiday is set in addition.

    @param[in] city a city holiday information to initialize(multi set is available with vector)
*/
LAPriceDataCalendar::LAPriceDataCalendar(LAStringVector city)
	: LAPriceDataType(DATA_CALENDAR), mpCalendar(0)
{
	LAMathCalendarSet	calendarSet;

	int		i = -1;
	try
	{
		if (city.size() > 1)
		{
			//LAMathCalendar		cal;
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
	catch(LACoreError& e)
	{
		// exception
		LAString	msg("InvalidData for LAPriceDataCalendar : from ");
		if (i >= 0) {
			msg += city[i];
		}
		throw e += LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	setNull(false);
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
LAPriceDataCalendar::LAPriceDataCalendar(const LAPriceDataCalendar& attr)
				: LAPriceDataType(DATA_CALENDAR), mCalendarSet(attr.mCalendarSet), 
				mCity(attr.mCity), mpCalendar(0)
{
	setNull(attr.isNull());
};

/*!
    @brief destructor
*/
LAPriceDataCalendar::~LAPriceDataCalendar(void) 
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
LAPriceDataType*
LAPriceDataCalendar::clone() const
{
    try {
    	LAPriceDataCalendar*	pAttr = new LAPriceDataCalendar(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief calendar city name in the string representation
    
    separated by a delimiter when use multi holiday cities (e.g. "NY:LDN:TKY")

    @return calendar city name separated by a delimiter
*/
LAString
LAPriceDataCalendar::convertToString(void) const
{
	LAString	ret;
	if (isNull())
	{
		ret = NULL_STR;
	}
	else
	{
		ret = "";
		if (mCity.size())
		{
			set<LAString>::const_iterator it = mCity.begin();
			set<LAString>::const_iterator it_end = mCity.end();
			
			for (it_end--;it != it_end; ++it)
			{
				ret += *it;
				ret += DATA_COLL_DEL;
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
const LAMathCalendar &
LAPriceDataCalendar::getCalendar(const bool isCache) const
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
			mpCalendar = new LAMathCalendar;
		}
	}
	else
	{
		mpCalendar = new LAMathCalendar;
	}
	set<const LAMathCalendar *>::const_iterator it = mCalendarSet.begin();
	while (it != mCalendarSet.end())
	{
		(*mpCalendar) += *(*it);
		++it;
	}
	return *mpCalendar;
}

const set<const LAMathCalendar *>&
LAPriceDataCalendar::getCalendarSet(void)const
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
LADate				
LAPriceDataCalendar::getBusinessDay(const LADate& d, int s) const
{
	LADate	ret = d;
	LADate	shift_pos;
	int		count = 0;
	int		next_step = 1;
	long num = LAMath::abs(s);
	
	bool isHoliday = true;
	if (s == 0)
	{
		while (isHoliday)
		{
			isHoliday = false;
			set<const LAMathCalendar *>::const_iterator it = mCalendarSet.begin();
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
			set<const LAMathCalendar *>::const_iterator it = mCalendarSet.begin();
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
LAPriceDataCalendar::businessDays( const LADate& fromDate, const LADate& toDate ) const
{
    int nBusinessDays = 0;
    LADate tempDate = fromDate;
    do 
    {
        tempDate = LAPriceDataCalendar::getBusinessDay( fromDate, nBusinessDays );
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
LADate				
LAPriceDataCalendar::getEOMDay(const LADate& d) const
{
	LADate eomDate = d;
	eomDate.addDays(d.intervalToEndOfMonth());
	bool isHoliday = true;
	while (isHoliday)
	{
		isHoliday = false;
		set<const LAMathCalendar *>::const_iterator it = mCalendarSet.begin();
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
LADate				
LAPriceDataCalendar::getEOWDay(const LADate& d) const
{
	LADate eowDate = d;
	LADayOfWeekEnum w = d.dayOfWeek();
	eowDate.addDays(FRI - w); //set to Friday
	bool isHoliday = true;
	while (isHoliday)
	{
		isHoliday = false;
		set<const LAMathCalendar *>::const_iterator it = mCalendarSet.begin();
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

	@param[in] d LADate input date

	@return first business day of the month within which the input date falls
*/
LADate
LAPriceDataCalendar::getBOMDay(const LADate& d) const
{
	LADate bomDate = d;
	bomDate.addDays(d.intervalToStartOfMonth());
	bool isHoliday = true;
	while (isHoliday)
	{
		isHoliday = false;
		set<const LAMathCalendar *>::const_iterator it = mCalendarSet.begin();
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
LAPriceDataCalendar::convertFromString(const LAString& str)
{
	
	LAMathCalendarSet		calendarSet;
	mCity.clear();
	mCalendarSet.clear();
	setNull();
		
	LAString data;
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
	vector<LAString> tokens = data.toToken( DATA_COLL_DEL );
	vector<LAString>::iterator it = tokens.end();
	try {
		if (tokens.size() > 1)
		{
			for (it = tokens.begin();it != tokens.end(); ++it)
			{
				//LAMathCalendar	cal;
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
	catch (LACoreError& e)
	{
		setNull(true);
		// exception
		LACoreInvalidData invaldEx(e.getMsg(), __FILE__, __LINE__);
		LAString	msg("Invalid Calendar: Unable to load calendar - ");
		if (it != tokens.end())
		{
			msg += *it;
		}
		invaldEx.addMsg(msg.getCString());
		throw invaldEx;
	}
	
	// notify of the change
	update();
}

/*
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
LAPriceDataType&
LAPriceDataCalendar::assignment(const LAPriceDataType& a)
{
	if (this == &a) return *this;
	if (a.getType() != DATA_CALENDAR)
	{
		// exception
		LAString	msg("Assignment error for LAPriceDataCalendar : from ");
		msg += LAString(a.getType());
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	update();
	//mCalendar = dynamic_cast<const LAPriceDataCalendar&>(a).mCalendar;
	mCalendarSet = dynamic_cast<const LAPriceDataCalendar&>(a).mCalendarSet;
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
LAPriceDataCalendar::compare(const LAPriceDataType& a) const
{
	if (a.getType() != DATA_CALENDAR)
	{
		// exception
		LAString	msg("Compare error for LAPriceDataCalendar : from ");
		msg += LAString(a.getType());
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
	return mCity.size() - dynamic_cast<const LAPriceDataCalendar&>(a).mCity.size(); 
}
