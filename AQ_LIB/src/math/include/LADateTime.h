#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LADate.h"


/*! 
    @brief Class which provides date or time functions.

    This class has time information in addition to the date information that LADate class has.
*/
class LADateTime : public LADate
{
public:
//  LIFECYCLE                    
    // Set default date is 1/01/01
    // default constructor
    LADateTime(void);
    // constructor
	explicit LADateTime(const char_t*   datetime, const char_t*   format="YYYYMMDD HH:MI:SS");
    // copy constructor
    LADateTime(const LADateTime& date);
    // destructor
    virtual ~LADateTime();

//  OPERATION
    // set the date according to a format
    virtual void        setDate(const char_t*   date, 
                                const char_t*   format="YYYYMMDD HH:MI:SS");

    // set the system date
    virtual void        setSystemDate(void);
    // set hour
    void                setHour(unsigned int hour);
    // set minute
    void                setMinute(unsigned int min);
    // set second
    void                setSecond(unsigned int sec);
    // add hours
    void                addHours(const int hours);
    // add minutes
    void                addMinutes(const int mins);
    // add seconds
    void                addSeconds(const int seconds);
   
//  QUERY
    // return hour
    int                 getHour(void) const;
    // return minute
    int                 getMinute(void) const;
    // return second
    int                 getSecond(void) const;

    // the date and time according to the format
    virtual LAString    stringWithFormat(const char_t*   format="YYYYMMDD HH:MI:SS") const;

    // make a comparison of time. Returns a positive value when their own is the new time.
    int					cmp(const LADateTime& rTime) const;

	// OPERATOR
    // relational operator
    bool                operator==(const LADateTime& a) const {return cmp(a) == 0;}
    // relational operator
    bool                operator!=(const LADateTime& a) const {return cmp(a) != 0;}
    // relational operator
    bool                operator<=(const LADateTime& a) const {return cmp(a) <= 0;}
    // relational operator
    bool                operator>=(const LADateTime& a) const {return cmp(a) >= 0;}
    // relational operator
    bool                operator>(const LADateTime& a)  const {return cmp(a) > 0;}
    // relational operator
    bool                operator<(const LADateTime& a)  const {return cmp(a) < 0;}

protected:
    // set the date according to a format
    void				formatWithString(const char_t *st,  const char_t *format);
    // get the date according to a format
    void				formatWithLong(LAString& str,
                                       const char_t *format) const;

    // shallow copy of the object
    void				copy(const LADateTime& rTime);

private:

    unsigned short     mHour;      // hour(023)
    unsigned short     mMinute;    // minute(059)
    unsigned short     mSecond;    // second(059)
};
