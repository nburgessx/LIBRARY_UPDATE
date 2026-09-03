/*! @file
    @brief Implementation of calendar class to expess holiday information.

*/


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "AQLMathCalendar.h"
#include "AQLString.h"
#include <deque>
#include <set>
#include <algorithm>
#include <string.h>

#ifdef __HAS_MIC__

#endif

using namespace std;

#ifdef __HAS_MIC__
common_lib::StaticMutex AQLMathCalendar::mMutex;
#endif
// used by isHoliday() to decide how many holidays to create. created HOLIDAY_BUFFERING_SIZE * 2
#define HOLIDAY_BUFFERING_SIZE (365)
#ifdef __MDEBUG__
static AQLDate MIMIMUM_DATE("19500101");
#endif

/*!
    @brief Structure for storing information (such as Happy Mondays) of the indefinite holiday date

     hold the information of X Mon, Y of the week and  Z day of the week
 
*/
struct AQLFlowDate
{
    int                 mFMonth;  // month(112)
    int                 mFWeek;   // week(15)
    AQLDayOfWeekEnum              mFWeekly; // day of the week(Sum=0, Mon=1, ..., Sat=6)
    // relational operator
    bool operator <(const AQLFlowDate& d) const
    {
        return (mFMonth < d.mFMonth ? true :
                (mFMonth == d.mFMonth ? false : 
                 (mFWeek < d.mFWeek ? true : 
                 (mFWeek == d.mFWeek ? false :
                 mFWeekly - d.mFWeekly < 0 ? true : false))));
    }
};

/*!
    @brief structure for storing holiday information
*/
struct AQLMathCalendarInfo
{
    set<AQLDayOfWeekEnum>         mWeekly;   // holiday of day of the week (day X weekly)
    set<AQLString>       mDays;     // holiday of month and date(X year Y days yearly)
    set<AQLFlowDate>     mFlowDate; // indefinite holiday date(X month Y week Z day of the week)
    set<AQLDate>         mDate;     // holiday date(X year Y month X day)
};


/*!
   Internal class to represent the data for holiday information.
    (outside the scope of the specification for Doxygen)
*/
class AQLMathCalendarData
{
public:
//  LIFECYCLE
    // default constructor
    AQLMathCalendarData(void){ mpcalInfo = NULL;};
    // copy constructor
    AQLMathCalendarData(const AQLMathCalendarData& cal);
    // destructor
    ~AQLMathCalendarData(void);

//  QUERY
    // count number of holidays between startDate and endDate (including both ends)
    int                 countHoliday(const AQLDate& startDate, 
                                     const AQLDate& endDate) const;
    // check whether holidya or not
    bool                isHoliday(const AQLDate& date) const;

//  OPERATION
    // set the specified day of the weekly holiday
    void                setWeekly(const AQLDayOfWeekEnum weekly);
    // set the specified day of holiday every year
    void                setDays(const AQLString& days);
    // set the variable specified day of holiday every year
    void                setFlowDate(const int month, const int week,
                                    const AQLDayOfWeekEnum weekly);
    // set the date specified by the holiday
    void                setDate(const AQLDate& date);

//  OPERATOR
    // copy the right-hand side to the left-hand side 
    const AQLMathCalendarData&   operator=(const AQLMathCalendarData& cal);
    // integrate the left and right sides
    const AQLMathCalendarData&   operator+=(const AQLMathCalendarData& cal);

private:
    AQLMathCalendarInfo*             mpcalInfo;  // holiday info
    mutable deque<AQLDate>       mholiday;
    mutable AQLDate              mStart;
    mutable AQLDate              mEnd;

    // create hodilday data
    inline void createHolidayData(const AQLDate& startDate, 
                                   const AQLDate& endDate) const;
    // add holiday data specified by the day of the week
    void addWeeklyHoliday(set<AQLDate>& list,
                          const AQLDate& startDate,
                          const AQLDate& endDate) const;
    // add holiday data specified by the day of the year
    void addDaysHoliday(set<AQLDate>& list,
                        const AQLDate& startDate,
                        const AQLDate& endDate) const;
    // add indefinite holiday data
    void addFlowDateHoliday(set<AQLDate>& list,
                            const AQLDate& startDate,
                            const AQLDate& endDate) const;
    // add holiday data specifed by year, month and date
    void addDateHoliday(set<AQLDate>& list,
                        const AQLDate& startDate,
                        const AQLDate& endDate) const;
    // search holiday data
    int getPointholiday(int& startIndex, int& endIndex, 
                        const AQLDate& date) const;
    // check weekly data
    bool checkWeeklyData(const AQLString weekly);

    // check days
    bool checkDays(const AQLString days);

};
////////////////////////////////////////////////////////////////////////////
// AQLMathCalendarData

//--------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------
AQLMathCalendarData::AQLMathCalendarData(const AQLMathCalendarData& cal)
{
    mpcalInfo = NULL;
    try
    {
        // copy of calendar 
        mpcalInfo = new AQLMathCalendarInfo(*cal.mpcalInfo);
        // add holiday data
        mholiday = cal.mholiday;
        // fix range of calendar
        mStart = cal.mStart;
        mEnd   = cal.mEnd;
    }
    catch(...)
    {
        // exception
        if (mpcalInfo)
        {
            delete mpcalInfo;
            mpcalInfo = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}
//--------------------------------------------------------------------------
//  destructor
//--------------------------------------------------------------------------
AQLMathCalendarData::~AQLMathCalendarData()
{ 
    if (mpcalInfo)
    {
        delete mpcalInfo;
    }
}
// QUERY
//--------------------------------------------------------------------------
// count holidays
//--------------------------------------------------------------------------
int 
AQLMathCalendarData::countHoliday(
    const AQLDate& startDate,
    const AQLDate& endDate) const
{
    int     startIndexA, endIndexA, startIndexB, endIndexB;
    AQLDate  sDate(startDate), eDate(endDate);
    // order of the date
    if (startDate > endDate)
    {
        sDate = endDate;
        eDate = startDate;
    }
    // create holiday data
    createHolidayData(sDate, eDate);
    // excluded data
    if (mholiday.size() == 0)
    {
        return 0;
    }
    AQLDate s = mholiday.front(); 
    AQLDate e = mholiday.back();
    if (s > eDate)
    {
        return 0;
    }   
    if (e < sDate)
    {
        return 0;
    }
    // search
    if (sDate <= s )
    {
        startIndexA = 0;
        endIndexA = 0;
    }
    else 
    {
        startIndexA = 0; 
        endIndexA = mholiday.size() - 1;
        getPointholiday(startIndexA, endIndexA, sDate);
    }
    if (eDate >= e)
    {
        startIndexB = endIndexB =mholiday.size() - 1;
    }
    else
    {
        startIndexB = 0;
        endIndexB = mholiday.size() - 1;
        getPointholiday(startIndexB, endIndexB, eDate);
    }
    // calculate number of days from Index
    if (startIndexB == endIndexB || startIndexA == endIndexA)
    {
        return startIndexB - endIndexA + 1;
    }
    return startIndexB - startIndexA;
}
//--------------------------------------------------------------------------
//  search holiday date(recursive)
//--------------------------------------------------------------------------
int 
AQLMathCalendarData::getPointholiday(
    int& startIndex, 
    int& endIndex, 
    const AQLDate& date) const
{
    int half, ans;
    half = (endIndex + startIndex) / 2;
    AQLDate  wkDate;

    if (mholiday[half] == date)
    {
        startIndex = half;
        endIndex = half;
        return half;
    } 
    else if (mholiday[half] < date)
    {
        startIndex = half;
        if (endIndex - startIndex == 1)
        {
            if (mholiday[startIndex] > date){
                startIndex--;
                endIndex--;
            }
            return half;
        }
    }
    else if (mholiday[half] > date)
    {
        endIndex = half;

        if (mholiday[startIndex] == date)
        {
            endIndex = startIndex;
            return half;
        }
        if (mholiday[startIndex] < date)
        {
            if (endIndex - startIndex == 1 )
            {
                return half;
            }
        }
    }
    ans = getPointholiday(startIndex, endIndex, date);
    return ans;
}
//--------------------------------------------------------------------------
// check whether holiday or not
//--------------------------------------------------------------------------
bool 
AQLMathCalendarData::isHoliday(
    const AQLDate& date) const
{
    // if necessary, prepare one more year holiday information
    if (date < mStart || date > mEnd)
    {
        AQLDate  endDate(date);
        endDate.addDays(HOLIDAY_BUFFERING_SIZE);
        createHolidayData(date, endDate);
    }
    
    // search
    return binary_search(mholiday.begin(), mholiday.end(), date);

}

// OPERATION
//--------------------------------------------------------------------------
// set the specified day of the weekly holiday
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::setWeekly(
    const AQLDayOfWeekEnum weekly)
{
    try
    {
        if (mpcalInfo == NULL)
        {
            mpcalInfo = new AQLMathCalendarInfo;
        }
        mpcalInfo->mWeekly.insert(weekly);
    }
    catch(...)
    {
        // exception
        if (mpcalInfo)
        {
            delete mpcalInfo;
            mpcalInfo = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

//--------------------------------------------------------------------------
//  set the specified day of holiday every year
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::setDays(
    const AQLString& days)
{
    if (days.size() == 0 || !checkDays(days))
    {
        // exception
        AQLString    msg("Invalid Data : Param Error ");
        if (days.size() == 0)
        {
            msg += "(Days.size() == 0)";    
            
        }
        else
        {
            msg += "(" + days + ") ";
        }
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    try
    {
        if (mpcalInfo == NULL)
        {
            mpcalInfo = new AQLMathCalendarInfo;
        }
        mpcalInfo->mDays.insert(days);
    }
    catch(...)
    {
        // exception
        if (mpcalInfo)
        {
            delete mpcalInfo;
            mpcalInfo = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

//--------------------------------------------------------------------------
//  set the variable specified day of holiday every year
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::setFlowDate(
    const int month,
    const int week,
    const AQLDayOfWeekEnum weekly)
{

    if (1 > month || month > 12 || 1 > week || week > 6 )
    {
        // exception
        char    errorData[64];
        memset (errorData, 0x00, sizeof(errorData));
        AQLString    msg("Invalid Data : Param Error ");
        if ( 1 > month || month > 12)
        {
            SPRINTF(errorData, "month = %d ", month);
            msg += errorData;
        }
        else
        {
            SPRINTF(errorData, "week = %d ", week);
        }
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    try
    {
        if (mpcalInfo == NULL)
        {
            mpcalInfo = new AQLMathCalendarInfo;
        }
        AQLFlowDate flow; 
        flow.mFMonth = month;
        flow.mFWeek = week;
        flow.mFWeekly = weekly;
        mpcalInfo->mFlowDate.insert(flow);
    }
    catch(...)
    {
        // exception
        if (mpcalInfo)
        {
            delete mpcalInfo;
            mpcalInfo = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

//--------------------------------------------------------------------------
//  set the date specified by the holiday
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::setDate(
    const AQLDate& date)
{
    try
    {
        if (mpcalInfo == NULL)
        {
            mpcalInfo = new AQLMathCalendarInfo;
        }
        mpcalInfo->mDate.insert(date);
    }
    catch(...)
    {
        // exception
        if (mpcalInfo)
        {
            delete mpcalInfo;
            mpcalInfo = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

// OPERATOR
//--------------------------------------------------------------------------
//  object copy
//--------------------------------------------------------------------------
const AQLMathCalendarData& 
AQLMathCalendarData::operator=(
    const AQLMathCalendarData& cal)
{
    try
    {
        if (mpcalInfo == NULL)
        {
            mpcalInfo = new AQLMathCalendarInfo;
        }
        // data copy
        (*mpcalInfo) = *(cal.mpcalInfo);
        mholiday = cal.mholiday;
        mStart = cal.mStart;
        mEnd = cal.mEnd;
    }
    catch(...)
    {
        // exception
        if (mpcalInfo)
        {
            delete mpcalInfo;
            mpcalInfo = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
    return *this;
}

//--------------------------------------------------------------------------
//  additional object copy
//--------------------------------------------------------------------------
const AQLMathCalendarData& 
AQLMathCalendarData::operator+=(
    const AQLMathCalendarData& cal)
{
    set<AQLDayOfWeekEnum>::iterator       pw;
    set<AQLString>::iterator     p;
    set<AQLFlowDate>::iterator   p2;
    set<AQLDate>::iterator       p3;
// unused.    deque<AQLDate>::iterator     p4;
    try
    {
        if (mpcalInfo == NULL)
        {
            mpcalInfo = new AQLMathCalendarInfo;
        }
        // add definition data
        if (cal.mpcalInfo->mWeekly.size())
        {
            pw = cal.mpcalInfo->mWeekly.begin();
            for (;pw != cal.mpcalInfo->mWeekly.end(); ++pw)
            {
                mpcalInfo->mWeekly.insert(*pw);
            }
        }
        if (cal.mpcalInfo->mDays.size())
        {
            p = cal.mpcalInfo->mDays.begin();
            for (;p != cal.mpcalInfo->mDays.end();++p)
            {
                mpcalInfo->mDays.insert(*p);
            }
        }
        if (cal.mpcalInfo->mFlowDate.size())
        {
            p2 = cal.mpcalInfo->mFlowDate.begin();
            for (;p2 != cal.mpcalInfo->mFlowDate.end();++p2)
            {
                mpcalInfo->mFlowDate.insert(*p2);
            }
        }
        if (cal.mpcalInfo->mDate.size())
        {
            p3 = cal.mpcalInfo->mDate.begin();
            for (;p3 != cal.mpcalInfo->mDate.end();++p3)
            {
                mpcalInfo->mDate.insert(*p3);
            }
        }
        // initialize
        // copy only definition data
        mStart = AQLDate();
        mEnd = AQLDate();
        mholiday.clear();
    }
    catch(AQLCoreError&)
    {
        throw;
    }
    catch(...)
    {
        // exception
        if (mpcalInfo)
        {
            delete mpcalInfo;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
    return *this;
}

//--------------------------------------------------------------------------
// create hodilday data
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::createHolidayData(
    const AQLDate& startDate, 
    const AQLDate& endDate) const
{
    // whether need to create or not
    AQLDate def;
    if (mStart != def && mStart <= startDate && mEnd >= endDate) return;
#ifdef __MDEBUG__
    // 
    if (startDate < MIMIMUM_DATE)
    {
        AQLString msg("Input date[");
        msg += startDate.stringWithFormat(" YYYYMMDD ]");
        msg += " is before the mimimun_date[19500101]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
#endif
    // whether there is create info
    if (mpcalInfo == NULL)
    {
        return;
    }
    // save temporary created data
    set<AQLDate>                     cashHoliday; 
    set<AQLDate>::iterator           it;
    set<AQLDate>::reverse_iterator   rit;
    // Create start
    if (mStart == def) // first call
    {
        addDateHoliday(cashHoliday, startDate, endDate);
        addDaysHoliday(cashHoliday, startDate, endDate);
        addFlowDateHoliday(cashHoliday, startDate, endDate);
        addWeeklyHoliday(cashHoliday, startDate, endDate);
        it = cashHoliday.begin();
        for (;it != cashHoliday.end();++it)
        {
            mholiday.push_back(*it);
        }
    }
    else
    {
        // in case of forward
        if (mStart > startDate)
        {
            AQLDate front(mStart);
            front.addDays(-1);      // duplication prevention
            addDateHoliday(cashHoliday, startDate, front);
            addDaysHoliday(cashHoliday, startDate, front);
            addFlowDateHoliday(cashHoliday, startDate, front);
            addWeeklyHoliday(cashHoliday, startDate, front);
            rit = cashHoliday.rbegin();
            for (;rit != cashHoliday.rend();++rit)
            {
                mholiday.push_front(*rit);
            }
        }
        cashHoliday.clear();
        // in case of back part
        if (mEnd < endDate)
        {
            AQLDate back(mEnd);
            back.addDays(1);        // duplication prevention
            addDateHoliday(cashHoliday, back, endDate);
            addDaysHoliday(cashHoliday, back, endDate);
            addFlowDateHoliday(cashHoliday, back, endDate);
            addWeeklyHoliday(cashHoliday, back, endDate);
            it = cashHoliday.begin();
            for (;it != cashHoliday.end();++it)
            {
                mholiday.push_back(*it);
            }
        }
    }
    if (mStart==def || mStart > startDate) mStart = startDate;
    if (mEnd < endDate) mEnd = endDate;
}

//--------------------------------------------------------------------------
// add holiday data specified by the day of the week
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::addWeeklyHoliday(
    set<AQLDate>& list,
    const AQLDate& startDate,
    const AQLDate& endDate) const
{
    // whether to create or not
    // return if no holiday
    if (mpcalInfo->mWeekly.size() == 0)
    {
        return;
    }
    // start
    AQLDate          wkdate;
    set<AQLDayOfWeekEnum>::iterator       p;
    p = mpcalInfo->mWeekly.begin();
    for (;p != mpcalInfo->mWeekly.end();++p)
    {
        wkdate = startDate;
        AQLDayOfWeekEnum sw = startDate.dayOfWeek();
        if (sw != *p){
            int sub;
            // from the number of days difference between the specified day,
			//calculate the date of the specified day of the week
            if ( sw > *p)
            {   // plus 7 days of the week if it is greater than startDate
                sub = 7 + *p - sw;
            }
            else
            {
                sub = *p - sw;
            }
            wkdate.addDays(sub);
        }
        // from the minimub date of the specified day, set the date
        for (;wkdate <= endDate;)
        {
            list.insert(wkdate);
            wkdate.addDays(7);
        }
    }
}

//--------------------------------------------------------------------------
//  add holiday data specified by the day of the year
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::addDaysHoliday(
    set<AQLDate>& list,
    const AQLDate& startDate,
    const AQLDate& endDate) const
{
    // whether to create or not
    if (!mpcalInfo->mDays.size())
    {
        return;
    }
    // start
    AQLDate          wkdate;
    AQLString        mmdd;
    int             sYear, eYear;
    set<AQLString>::iterator     p;

    // set start and end year
    sYear = startDate.yearOfEra();
    eYear = endDate.yearOfEra();

    for(int i = 0;i <= eYear - sYear;i++)
    {
        p = mpcalInfo->mDays.begin();
        for(;p != mpcalInfo->mDays.end();++p)
        {
            mmdd = *p;
            wkdate.setYear(sYear + i);
            wkdate.setMonth((mmdd[0]-'0')*10 + mmdd[1] - '0');
            wkdate.setDay((mmdd[3]-'0')*10 + mmdd[4]-'0');
            if (wkdate <= endDate && wkdate >= startDate)
            {
                list.insert(wkdate);
            }
        }
    }
}
//--------------------------------------------------------------------------
// add indefinite holiday data
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::addFlowDateHoliday(
    set<AQLDate>& list,
    const AQLDate& startDate,
    const AQLDate& endDate) const
{
    // whether to create or not
    if (!mpcalInfo->mFlowDate.size())
    {
        return;
    }
    // start
    AQLDate          wkdate;
    set<AQLFlowDate>::iterator       p;
// unused.    set<AQLDate>::iterator           end = list.end();
    int             sYear, eYear, weekly, day;

    sYear = startDate.yearOfEra();
    eYear = endDate.yearOfEra();

    p = mpcalInfo->mFlowDate.begin();
    for (;p != mpcalInfo->mFlowDate.end();++p)
    {
        for (int i = 0;i <= eYear - sYear;i++)
        {
            // get the selected day of the week (store into j)
            // get day of the week of the beginning of the month
            wkdate.setYear(sYear+i);
            wkdate.setMonth(p->mFMonth);
            wkdate.setDay(1);
            weekly = wkdate.dayOfWeek();
            if (weekly > p->mFWeekly)
            {
                day = 7 * p->mFWeek - (weekly - p->mFWeekly - 1);
            }
            else
            {
                day = 7 * (p->mFWeek - 1) + p->mFWeekly - weekly +1;
            }
            wkdate.setDay(day);
            if (wkdate <= endDate && wkdate >= startDate)
            {
                list.insert(wkdate);
            }
        }
    }
}
//--------------------------------------------------------------------------
//  add holiday data specifed by year, month and date
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::addDateHoliday(
    set<AQLDate>& list,
    const AQLDate& startDate,
    const AQLDate& endDate) const
{
    set<AQLDate>::iterator       p;
    
    p = mpcalInfo->mDate.begin();
    for (;p != mpcalInfo->mDate.end();++p)
    {
        if (*p < startDate || *p > endDate) continue;
        list.insert(*p);
    }
}

/*!
    @brief check days

    check 1-12 in case of month and 1-31 in case of days respectively
    @para [in] days "YY/MM","YY-MM", "YY.MM" etc.(YY and MM delimiter can be anything if a single-byte character)
	@retval true correct day and month string
    @retval false otherwise
*/
bool 
AQLMathCalendarData::checkDays(
    const AQLString days)
{
    AQLString    mm,dd;

    if (days.size() != 5)
    {
        return false;
    }
    mm = days.subString(0, 1);
    dd = days.subString(3, 4);

    if( 1 > mm.getIntValue() || mm.getIntValue() > 12)
    {
        return false;
    }

    if( 1 > dd.getIntValue() || dd.getIntValue() > 31)
    {
        return false;
    }
    return true;
}



///////////////////////// AQLMathCalendar class//////////////////////////////////
/*!
    @brief constructor
*/
AQLMathCalendar::AQLMathCalendar()
{
    mpCalData = NULL;
    mpRefCount = NULL;

    try
    {
        mpCalData = new AQLMathCalendarData;
        mpRefCount = new int(1);
    }
    catch(...)
    {
        // exception
        if (mpCalData)
        {
            delete mpCalData;
            mpCalData = NULL;
        }
        if (mpRefCount)
        {
            delete mpRefCount;
            mpRefCount = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

/*!
    @brief copy constructor
    @param[in] calHolder original holiday object
*/
AQLMathCalendar::AQLMathCalendar(const AQLMathCalendar& calHolder)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    mpCalData = NULL;
    mpRefCount = NULL;
    copy(calHolder);
}

/*!
    @brief destructor
*/
AQLMathCalendar::~AQLMathCalendar()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    clear();
}


/*!
    @brief assignment operator

    copy the right-hand side to the left-hand side
*/
const AQLMathCalendar& 
AQLMathCalendar::operator=(const AQLMathCalendar& cal)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    copy(cal);
    return *this;
}

/*!
    @brief addition assignment operator

    additionally copy the right-hand side to the left-hand side
*/
const AQLMathCalendar& 
AQLMathCalendar::operator+=(const AQLMathCalendar& cal)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    makeUnShared();
    try{
        *mpCalData += *(cal.mpCalData);
    }
    catch(AQLCoreError& e)
    {
        AQLCoreError ex("Error at AQLMathCalendar operator+= Method", __FILE__, __LINE__);
        ex += e;
        throw ex;
    }
    catch(...)
    {
        if (mpRefCount)
        {
            delete mpRefCount;
            mpRefCount = NULL;
        }
        if (mpCalData)
        {
            delete mpCalData;
            mpCalData = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
    return *this;
}

/*!
    @brief get the number of holiday (including both start and end) for a specified period of time

    @param[in] startDate start date
    @param[in] endDate   end date

    @return the number of holiday
*/
int 
AQLMathCalendar::countHoliday(const AQLDate& startDate, const AQLDate& endDate) const
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    int ret =   mpCalData->countHoliday(startDate, endDate);
	return ret;
}

/*!
    @brief check whether the selected day is holiday or not

    @param[in] date checked day

    @retval true holiday 
    @retval false not holiday
*/
bool 
AQLMathCalendar::isHoliday(const AQLDate& date) const
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    bool ret =  mpCalData->isHoliday(date);
	return ret;
}

/*!
    @brief the specified day of the week is set to the holiday

    @param[in] weekly day of the week to be set(SUN, MON, ...)
*/
void 
AQLMathCalendar::setWeekly(const AQLDayOfWeekEnum weekly)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    makeUnShared();
    mpCalData->setWeekly(weekly);
}

/*!
    @brief the specified day is set to the holiday 

    @param[in] days days to be set("MM/DD", "MM-DD", "MM.DD" etc)
*/
void 
AQLMathCalendar::setDays(const AQLString& days)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    makeUnShared();
    mpCalData->setDays(days);
}

/*!
    @brief the specified multi days are set to the holiday

    @param[in] daysVector multi days to be set("MM/DD", "MM-DD", "MM.DD" etc)
*/
void 
AQLMathCalendar::setDays(const AQLStringVector& daysVector)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    makeUnShared();
    for (unsigned int i = 0; i < daysVector.size(); i++){
        mpCalData->setDays(daysVector[i]);
    }
}

/*!
    @brief set the date of X month Y of the week and Z day of the week on holidays(indefinite holiday)

    @param[in] month  month 
    @param[in] week   week
    @param[in] weekly day of the week
*/
void 
AQLMathCalendar::setFlowDate(const int month, const int week, const AQLDayOfWeekEnum weekly)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    makeUnShared(); // one time action is enough for reselt of reference counter and release data sharing
    mpCalData->setFlowDate(month, week, weekly);
}

/*!
    @brief the specified date is set to the holiday

    @param[in] date date to be set
*/
void 
AQLMathCalendar::setDate(const AQLDate& date)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    makeUnShared();
    mpCalData->setDate(date);
}

/*!
    @brief the specified multi dates are set to the holiday

    @param[in] dateVector multi dates to be set
*/
void 
AQLMathCalendar::setDate(const DateVector& dateVector)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    makeUnShared(); // one time action is enough for reselt of reference counter and release data sharing
    for (unsigned int i = 0; i < dateVector.size(); i++){
        mpCalData->setDate(dateVector[i]);
    }
}

/*!
    @brief free the memory acquired

    free the memory obtained from holiday information
*/
void 
AQLMathCalendar::clear()
{
    if (mpRefCount != NULL && (*mpRefCount)-- == 1)
    {
        delete mpCalData;
        delete mpRefCount;
    }
    mpCalData = NULL;
    mpRefCount = NULL;
}


/*!
    @brief shallow copy of the project
*/
void 
AQLMathCalendar::copy(const AQLMathCalendar& cal)
{
    if (this == &cal)
    {
        return;
    }
    clear();

    mpRefCount = cal.mpRefCount;
    ++(*mpRefCount);
    mpCalData = cal.mpCalData;
}

/*!
    @brief reset the reference counter of AQLMathCalendarData object that the object holds


	If you want to (shallow)copy AQLMathCalendar object by copy method, then
	you must share data string between the original and referenced objects.
	(reference counter is increment when copied)

	By using this method, the shared data string is released, the reference counter is reset to 1.
	When you use this method, string data is copied to another memory area, 
	and AQLMathCalendar two objects hold the string data in a separate region.
	This, AQLMathCalendar one of the objects even if you change the data by methods such as setDate(), 
	will not affect the data string of AQLMathCalendar the other object.

*/
void AQLMathCalendar::makeUnShared(void)
{
    try
    {
        if ((*mpRefCount) == 1)
        {
            return;
        }
        --(*mpRefCount);
        mpRefCount = NULL;
        mpRefCount = new int(1);
        mpCalData = new AQLMathCalendarData(*mpCalData);
    }
    catch(...)
    {
        if (mpRefCount)
        {
            delete mpRefCount;
            mpRefCount = NULL;
        }
        if (mpCalData)
        {
            delete mpCalData;
            mpCalData = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

/////////////////////////////////////////////////////////////////////////////


/*!
    @brief set city name and holiday information

    @param[in] cityName string representing city name
    @param[in] calendar holiday information corresponding to the city name
*/
void
AQLMathCalendarCollection::setCalendarData(const AQLString &cityName, const AQLMathCalendar &calendar)
{
    pair<map<AQLString, AQLMathCalendar>::iterator, bool> ret;

    ret = mCalendarList.insert( pair<AQLString, AQLMathCalendar>(cityName, calendar) );

    // if city name has been already registered
    if (! ret.second ){
        mCalendarList.erase(ret.first); // delete
        ret = mCalendarList.insert( pair<AQLString, AQLMathCalendar>(cityName, calendar) );
    }

    if (! ret.second ){
        AQLString msg = "Unable to register calendar data for ";
        msg += cityName;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
}

/*!
    @brief get AQLMathCalendar object from the specified city names

    @param[in] cityName string representing city name

    @return calendar holiday information corresponding to the city name
*/
const AQLMathCalendar& 
AQLMathCalendarCollection::getCalendarData(const AQLString& cityName)

{
    map<AQLString, AQLMathCalendar>::iterator     itr;
    
    itr = mCalendarList.find(cityName);
    if (itr == mCalendarList.end())
    {
        AQLString    msg = "#Error: Invalid Holiday Centre specified: ";
        msg += cityName;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
	mInqCityNames.insert(cityName);
    return itr->second;
}

/*!
    @brief get a list of city names

    @param[out] arr string representing city name
*/
void 
AQLMathCalendarCollection::getMemberNames(std::vector<AQLString>& arr)
{
    map<AQLString, AQLMathCalendar>::iterator     itr;
    for (itr = mCalendarList.begin(); itr != mCalendarList.end(); itr++)
    {
        arr.push_back(itr->first);
    }

}

/*!
    @brief get day of the week(enum value) from the string format, e.g. "SUN"

    @param[in] weekly ("SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT")

    @return AQLDayOfWeekEnum type data representing day of the week(SUN=0, MON=1, TUE=2, ..., SAT=6)

    @sa enum AQLDayOfWeekEnum
*/
AQLDayOfWeekEnum
AQLMathCalendarCollection::getWeeklyValue(const AQLString& weekly)
{
    AQLDayOfWeekEnum  week;
    if (weekly == "SUN")
    {
        week = SUN;
    }
    else if (weekly == "MON")
    {
        week = MON;
    }
    else if (weekly == "TUE")
    {
        week = TUE;
    }
    else if (weekly == "WED")
    {
        week = WED;
    }
    else if (weekly == "THU")
    {
        week = THU;
    }
    else if (weekly == "FRI")
    {
        week = FRI;
    }
    else if (weekly == "SAT")
    {
        week = SAT;
    }
    else 
    {
        AQLString    msg = "InvalidData Weekly fail data [";
        msg += weekly + "]";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
    return week;
}

/*!
    @brief get inquired city names

    @param[out] string representing city name
*/
void 
AQLMathCalendarCollection::getInquiredCityNames(std::vector<AQLString>& names) const
{
	names.clear();
	set<AQLString>::const_iterator it = mInqCityNames.begin();
	while (it != mInqCityNames.end())
	{
		names.push_back(*it);
		++it;
	}
}
