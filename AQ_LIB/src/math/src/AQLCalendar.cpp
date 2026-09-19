/*! @file
    @brief Implementation of calendar class to expess holiday information.
*/


#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#endif

#include "AQLCalendar.h"
#include "AQLString.h"
#include <deque>
#include <set>
#include <algorithm>
#include <memory>
#include <mutex>
#include <string.h>


using namespace std;

// used by isHoliday() to decide how many holidays to create. created HOLIDAY_BUFFERING_SIZE * 2
#define HOLIDAY_BUFFERING_SIZE (365)

/*!
    @brief Structure for storing information (such as Happy Mondays) of the indefinite holiday date
     hold the information of X Mon, Y of the week and  Z day of the week
*/
struct AQLFlowDate
{
    int                 mFMonth;  // month(112)
    int                 mFWeek;   // week(15)
    AQLDayOfWeekEnum    mFWeekly; // day of the week(Sum=0, Mon=1, ..., Sat=6)
    
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
    set<AQLDayOfWeekEnum>   mWeekly;   // holiday of day of the week (day X weekly)
    set<AQLString>          mDays;     // holiday of month and date(X year Y days yearly)
    set<AQLFlowDate>        mFlowDate; // indefinite holiday date(X month Y week Z day of the week)
    set<AQLDate>            mDate;     // holiday date(X year Y month X day)
};


/*!
   Internal class to represent the data for holiday information.
    (outside the scope of the specification for Doxygen)
*/
class AQLMathCalendarData
{
public:
    // default constructor
    AQLMathCalendarData(void){ pcalInfo_ = NULL;};
    
    // copy constructor
    AQLMathCalendarData(const AQLMathCalendarData& cal);
    
    // destructor
    ~AQLMathCalendarData(void);

    // count number of holidays between startDate and endDate (including both ends)
    int                 countHoliday(const AQLDate& startDate, const AQLDate& endDate) const;
    // check whether holidya or not
    bool                isHoliday(const AQLDate& date) const;

    // set the specified day of the weekly holiday
    void                setWeekly(const AQLDayOfWeekEnum weekly);
    
    // set the specified day of holiday every year
    void                setDays(const AQLString& days);
    
    // set the variable specified day of holiday every year
    void                setFlowDate(const int month, const int week, const AQLDayOfWeekEnum weekly);
    
    // set the date specified by the holiday
    void                setDate(const AQLDate& date);

    // copy the right-hand side to the left-hand side 
    const AQLMathCalendarData&   operator=(const AQLMathCalendarData& cal);
    
    // integrate the left and right sides
    const AQLMathCalendarData&   operator+=(const AQLMathCalendarData& cal);

private:

    AQLMathCalendarInfo*         pcalInfo_;  // holiday info
    mutable deque<AQLDate>       holiday_;
    mutable AQLDate              start_;
    mutable AQLDate              end_;

    // Guards holiday_/start_/end_, the lazy cache createHolidayData() populates on demand. This is
    // needed independently of AQLCalendar's own ownership model (see AQLCalendar.h): even a single
    // AQLMathCalendarData instance that is never copied or shared can still have isHoliday()/
    // countHoliday() called concurrently by multiple threads through one const AQLCalendar&
    // reference (e.g. AQLMathCalendarCollection::getCalendarData() handing out the same map entry
    // to many pricing threads at once) - both are const methods that silently mutate this cache on
    // first use, with zero synchronization before this fix. Not `mutable` itself - std::mutex's
    // lock()/unlock() are already usable through a const object; only the fields they protect
    // needed `mutable`. A single mutex covering both methods' entire bodies (not just
    // createHolidayData() internally) is deliberate: isHoliday()'s binary_search over holiday_ runs
    // after createHolidayData() returns, and would otherwise read it unlocked while another thread
    // could be concurrently expanding it.
    mutable std::mutex           dataMutex_;

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
    pcalInfo_ = NULL;
    try
    {
        // copy of calendar 
        pcalInfo_ = new AQLMathCalendarInfo(*cal.pcalInfo_);
        
        // add holiday data
        holiday_ = cal.holiday_;
        
        // fix range of calendar
        start_ = cal.start_;
        end_   = cal.end_;
    }
    catch(...)
    {
        // exception
        if (pcalInfo_)
        {
            delete pcalInfo_;
            pcalInfo_ = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}
//--------------------------------------------------------------------------
//  destructor
//--------------------------------------------------------------------------
AQLMathCalendarData::~AQLMathCalendarData()
{ 
    if (pcalInfo_)
    {
        delete pcalInfo_;
    }
}
//--------------------------------------------------------------------------
// count holidays
//--------------------------------------------------------------------------
int
AQLMathCalendarData::countHoliday(
    const AQLDate& startDate,
    const AQLDate& endDate) const
{
    std::lock_guard<std::mutex> lock(dataMutex_);
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
    if (holiday_.size() == 0)
    {
        return 0;
    }
    AQLDate s = holiday_.front(); 
    AQLDate e = holiday_.back();
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
        endIndexA = holiday_.size() - 1;
        getPointholiday(startIndexA, endIndexA, sDate);
    }
    if (eDate >= e)
    {
        startIndexB = endIndexB =holiday_.size() - 1;
    }
    else
    {
        startIndexB = 0;
        endIndexB = holiday_.size() - 1;
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

    if (holiday_[half] == date)
    {
        startIndex = half;
        endIndex = half;
        return half;
    } 
    else if (holiday_[half] < date)
    {
        startIndex = half;
        if (endIndex - startIndex == 1)
        {
            if (holiday_[startIndex] > date){
                startIndex--;
                endIndex--;
            }
            return half;
        }
    }
    else if (holiday_[half] > date)
    {
        endIndex = half;

        if (holiday_[startIndex] == date)
        {
            endIndex = startIndex;
            return half;
        }
        if (holiday_[startIndex] < date)
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
    std::lock_guard<std::mutex> lock(dataMutex_);
    // if necessary, prepare one more year holiday information
    if (date < start_ || date > end_)
    {
        AQLDate  endDate(date);
        endDate.addDays(HOLIDAY_BUFFERING_SIZE);
        createHolidayData(date, endDate);
    }
    
    // search
    return binary_search(holiday_.begin(), holiday_.end(), date);

}

//--------------------------------------------------------------------------
// set the specified day of the weekly holiday
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::setWeekly(
    const AQLDayOfWeekEnum weekly)
{
    try
    {
        if (pcalInfo_ == NULL)
        {
            pcalInfo_ = new AQLMathCalendarInfo;
        }
        pcalInfo_->mWeekly.insert(weekly);
    }
    catch(...)
    {
        // exception
        if (pcalInfo_)
        {
            delete pcalInfo_;
            pcalInfo_ = NULL;
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
        if (pcalInfo_ == NULL)
        {
            pcalInfo_ = new AQLMathCalendarInfo;
        }
        pcalInfo_->mDays.insert(days);
    }
    catch(...)
    {
        // exception
        if (pcalInfo_)
        {
            delete pcalInfo_;
            pcalInfo_ = NULL;
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
        if (pcalInfo_ == NULL)
        {
            pcalInfo_ = new AQLMathCalendarInfo;
        }
        AQLFlowDate flow; 
        flow.mFMonth = month;
        flow.mFWeek = week;
        flow.mFWeekly = weekly;
        pcalInfo_->mFlowDate.insert(flow);
    }
    catch(...)
    {
        // exception
        if (pcalInfo_)
        {
            delete pcalInfo_;
            pcalInfo_ = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

//--------------------------------------------------------------------------
//  set the date specified by the holiday
//--------------------------------------------------------------------------
void 
AQLMathCalendarData::setDate(const AQLDate& date)
{
    try
    {
        if (pcalInfo_ == NULL)
        {
            pcalInfo_ = new AQLMathCalendarInfo;
        }
        pcalInfo_->mDate.insert(date);
    }
    catch(...)
    {
        // exception
        if (pcalInfo_)
        {
            delete pcalInfo_;
            pcalInfo_ = NULL;
        }
        throw AQLCoreSystemError(__FILE__, __LINE__);
    }
}

//--------------------------------------------------------------------------
//  object copy
//--------------------------------------------------------------------------
const AQLMathCalendarData& 
AQLMathCalendarData::operator=(const AQLMathCalendarData& cal)
{
    try
    {
        if (pcalInfo_ == NULL)
        {
            pcalInfo_ = new AQLMathCalendarInfo;
        }
        // data copy
        (*pcalInfo_) = *(cal.pcalInfo_);
        holiday_ = cal.holiday_;
        start_ = cal.start_;
        end_ = cal.end_;
    }
    catch(...)
    {
        // exception
        if (pcalInfo_)
        {
            delete pcalInfo_;
            pcalInfo_ = NULL;
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
        if (pcalInfo_ == NULL)
        {
            pcalInfo_ = new AQLMathCalendarInfo;
        }
        // add definition data
        if (cal.pcalInfo_->mWeekly.size())
        {
            pw = cal.pcalInfo_->mWeekly.begin();
            for (;pw != cal.pcalInfo_->mWeekly.end(); ++pw)
            {
                pcalInfo_->mWeekly.insert(*pw);
            }
        }
        if (cal.pcalInfo_->mDays.size())
        {
            p = cal.pcalInfo_->mDays.begin();
            for (;p != cal.pcalInfo_->mDays.end();++p)
            {
                pcalInfo_->mDays.insert(*p);
            }
        }
        if (cal.pcalInfo_->mFlowDate.size())
        {
            p2 = cal.pcalInfo_->mFlowDate.begin();
            for (;p2 != cal.pcalInfo_->mFlowDate.end();++p2)
            {
                pcalInfo_->mFlowDate.insert(*p2);
            }
        }
        if (cal.pcalInfo_->mDate.size())
        {
            p3 = cal.pcalInfo_->mDate.begin();
            for (;p3 != cal.pcalInfo_->mDate.end();++p3)
            {
                pcalInfo_->mDate.insert(*p3);
            }
        }
        // initialize
        // copy only definition data
        start_ = AQLDate();
        end_ = AQLDate();
        holiday_.clear();
    }
    catch(AQLCoreError&)
    {
        throw;
    }
    catch(...)
    {
        // exception
        if (pcalInfo_)
        {
            delete pcalInfo_;
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
    if (start_ != def && start_ <= startDate && end_ >= endDate) return;
    // whether there is create info
    if (pcalInfo_ == NULL)
    {
        return;
    }
    // save temporary created data
    set<AQLDate>                     cashHoliday; 
    set<AQLDate>::iterator           it;
    set<AQLDate>::reverse_iterator   rit;
    // Create start
    if (start_ == def) // first call
    {
        addDateHoliday(cashHoliday, startDate, endDate);
        addDaysHoliday(cashHoliday, startDate, endDate);
        addFlowDateHoliday(cashHoliday, startDate, endDate);
        addWeeklyHoliday(cashHoliday, startDate, endDate);
        it = cashHoliday.begin();
        for (;it != cashHoliday.end();++it)
        {
            holiday_.push_back(*it);
        }
    }
    else
    {
        // in case of forward
        if (start_ > startDate)
        {
            AQLDate front(start_);
            front.addDays(-1);      // duplication prevention
            addDateHoliday(cashHoliday, startDate, front);
            addDaysHoliday(cashHoliday, startDate, front);
            addFlowDateHoliday(cashHoliday, startDate, front);
            addWeeklyHoliday(cashHoliday, startDate, front);
            rit = cashHoliday.rbegin();
            for (;rit != cashHoliday.rend();++rit)
            {
                holiday_.push_front(*rit);
            }
        }
        cashHoliday.clear();
        // in case of back part
        if (end_ < endDate)
        {
            AQLDate back(end_);
            back.addDays(1);        // duplication prevention
            addDateHoliday(cashHoliday, back, endDate);
            addDaysHoliday(cashHoliday, back, endDate);
            addFlowDateHoliday(cashHoliday, back, endDate);
            addWeeklyHoliday(cashHoliday, back, endDate);
            it = cashHoliday.begin();
            for (;it != cashHoliday.end();++it)
            {
                holiday_.push_back(*it);
            }
        }
    }
    if (start_==def || start_ > startDate) start_ = startDate;
    if (end_ < endDate) end_ = endDate;
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
    if (pcalInfo_->mWeekly.size() == 0)
    {
        return;
    }
    // start
    AQLDate          wkdate;
    set<AQLDayOfWeekEnum>::iterator       p;
    p = pcalInfo_->mWeekly.begin();
    for (;p != pcalInfo_->mWeekly.end();++p)
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
    if (!pcalInfo_->mDays.size())
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
        p = pcalInfo_->mDays.begin();
        for(;p != pcalInfo_->mDays.end();++p)
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
    if (!pcalInfo_->mFlowDate.size())
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

    p = pcalInfo_->mFlowDate.begin();
    for (;p != pcalInfo_->mFlowDate.end();++p)
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
    
    p = pcalInfo_->mDate.begin();
    for (;p != pcalInfo_->mDate.end();++p)
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



///////////////////////// AQLCalendar class//////////////////////////////////
/*!
    @brief constructor
*/
AQLCalendar::AQLCalendar()
    : pCalData_(std::make_unique<AQLMathCalendarData>())
{
}

/*!
    @brief copy constructor
    @param[in] calHolder original holiday object
*/
AQLCalendar::AQLCalendar(const AQLCalendar& calHolder)
    : pCalData_(std::make_unique<AQLMathCalendarData>(*calHolder.pCalData_))
{
}

// Move constructor - steals calHolder's data outright: no allocation, no copy of any holiday dates.
AQLCalendar::AQLCalendar(AQLCalendar&& calHolder) noexcept
    : pCalData_(std::move(calHolder.pCalData_))
{
}

/*!
    @brief destructor

    Declared here (not inline in the header, even though the body is now empty) because
    std::unique_ptr<AQLMathCalendarData>'s destructor needs that type complete at the point it is
    instantiated, and the header only forward-declares it.
*/
AQLCalendar::~AQLCalendar()
{
}


/*!
    @brief assignment operator

    copy the right-hand side to the left-hand side
*/
const AQLCalendar&
AQLCalendar::operator=(const AQLCalendar& cal)
{
    if (this != &cal)
    {
        pCalData_ = std::make_unique<AQLMathCalendarData>(*cal.pCalData_);
    }
    return *this;
}

// Move assignment - same rationale as the move constructor above.
AQLCalendar&
AQLCalendar::operator=(AQLCalendar&& cal) noexcept
{
    if (this != &cal)
    {
        pCalData_ = std::move(cal.pCalData_);
    }
    return *this;
}

/*!
    @brief addition assignment operator

    additionally copy the right-hand side to the left-hand side
*/
const AQLCalendar&
AQLCalendar::operator+=(const AQLCalendar& cal)
{
    // No more makeUnShared() - pCalData_ is always exclusively owned, nothing to detach from.
    try{
        *pCalData_ += *(cal.pCalData_);
    }
    catch(AQLCoreError& e)
    {
        AQLCoreError ex("Error at AQLCalendar operator+= Method", __FILE__, __LINE__);
        ex += e;
        throw ex;
    }
    catch(...)
    {
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
AQLCalendar::countHoliday(const AQLDate& startDate, const AQLDate& endDate) const
{
    int ret =   pCalData_->countHoliday(startDate, endDate);
	return ret;
}

/*!
    @brief check whether the selected day is holiday or not

    @param[in] date checked day

    @retval true holiday 
    @retval false not holiday
*/
bool 
AQLCalendar::isHoliday(const AQLDate& date) const
{
    bool ret =  pCalData_->isHoliday(date);
	return ret;
}

/*!
    @brief the specified day of the week is set to the holiday

    @param[in] weekly day of the week to be set(SUN, MON, ...)
*/
void 
AQLCalendar::setWeekly(const AQLDayOfWeekEnum weekly)
{
    pCalData_->setWeekly(weekly);
}

/*!
    @brief the specified day is set to the holiday 

    @param[in] days days to be set("MM/DD", "MM-DD", "MM.DD" etc)
*/
void 
AQLCalendar::setDays(const AQLString& days)
{
    pCalData_->setDays(days);
}

/*!
    @brief the specified multi days are set to the holiday

    @param[in] daysVector multi days to be set("MM/DD", "MM-DD", "MM.DD" etc)
*/
void 
AQLCalendar::setDays(const AQLStringVector& daysVector)
{
    for (unsigned int i = 0; i < daysVector.size(); i++){
        pCalData_->setDays(daysVector[i]);
    }
}

/*!
    @brief set the date of X month Y of the week and Z day of the week on holidays(indefinite holiday)

    @param[in] month  month 
    @param[in] week   week
    @param[in] weekly day of the week
*/
void 
AQLCalendar::setFlowDate(const int month, const int week, const AQLDayOfWeekEnum weekly)
{
    pCalData_->setFlowDate(month, week, weekly);
}

/*!
    @brief the specified date is set to the holiday

    @param[in] date date to be set
*/
void 
AQLCalendar::setDate(const AQLDate& date)
{
    pCalData_->setDate(date);
}

/*!
    @brief the specified multi dates are set to the holiday

    @param[in] dateVector multi dates to be set
*/
void 
AQLCalendar::setDate(const DateVector& dateVector)
{
    for (unsigned int i = 0; i < dateVector.size(); i++){
        pCalData_->setDate(dateVector[i]);
    }
}


/////////////////////////////////////////////////////////////////////////////


/*!
    @brief set city name and holiday information

    @param[in] cityName string representing city name
    @param[in] calendar holiday information corresponding to the city name
*/
void
AQLMathCalendarCollection::setCalendarData(const AQLString &cityName, const AQLCalendar &calendar)
{
    pair<map<AQLString, AQLCalendar>::iterator, bool> ret;

    ret = calendarList_.insert( pair<AQLString, AQLCalendar>(cityName, calendar) );

    // if city name has been already registered
    if (! ret.second ){
        calendarList_.erase(ret.first); // delete
        ret = calendarList_.insert( pair<AQLString, AQLCalendar>(cityName, calendar) );
    }

    if (! ret.second ){
        AQLString msg = "Unable to register calendar data for ";
        msg += cityName;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
}

/*!
    @brief get AQLCalendar object from the specified city names

    @param[in] cityName string representing city name

    @return calendar holiday information corresponding to the city name
*/
const AQLCalendar& 
AQLMathCalendarCollection::getCalendarData(const AQLString& cityName)

{
    map<AQLString, AQLCalendar>::iterator     itr;
    
    itr = calendarList_.find(cityName);
    if (itr == calendarList_.end())
    {
        AQLString    msg = "#Error: Invalid Holiday Centre specified: ";
        msg += cityName;
        throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
    }
	cityNames_.insert(cityName);
    return itr->second;
}

/*!
    @brief get a list of city names

    @param[out] arr string representing city name
*/
void 
AQLMathCalendarCollection::getMemberNames(std::vector<AQLString>& arr)
{
    map<AQLString, AQLCalendar>::iterator     itr;
    for (itr = calendarList_.begin(); itr != calendarList_.end(); itr++)
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
AQLMathCalendarCollection::getCityNames(std::vector<AQLString>& names) const
{
	names.clear();
	set<AQLString>::const_iterator it = cityNames_.begin();
	while (it != cityNames_.end())
	{
		names.push_back(*it);
		++it;
	}
}
