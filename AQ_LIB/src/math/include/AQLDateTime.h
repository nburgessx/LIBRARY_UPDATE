#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDate.h"

/*! 
    @brief Class which provides date or time functions.
    This class has time information in addition to the date information that AQLDate class has.
*/
class AQLDateTime : public AQLDate
{
public:
    
    // Set default date is 1/01/01
    // default constructor
    AQLDateTime(void);
    
    // constructor
	explicit AQLDateTime(const char_t* datetime, const char_t* format="YYYYMMDD HH:MI:SS");
    
    // copy constructor
    AQLDateTime(const AQLDateTime& date);
    
    // destructor
    virtual ~AQLDateTime();

    // set the date according to a format
    virtual void        setDate(const char_t* date, const char_t* format="YYYYMMDD HH:MI:SS");

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
   
    // return hour
    int                 getHour(void) const;
    // return minute
    int                 getMinute(void) const;
    // return second
    int                 getSecond(void) const;

    // the date and time according to the format
    virtual AQLString    stringWithFormat(const char_t*   format="YYYYMMDD HH:MI:SS") const;

    // make a comparison of time. Returns a positive value when their own is the new time.
    int					cmp(const AQLDateTime& rTime) const;

    // relational operator
    bool                operator==(const AQLDateTime& a) const {return cmp(a) == 0;}
    // relational operator
    bool                operator!=(const AQLDateTime& a) const {return cmp(a) != 0;}
    // relational operator
    bool                operator<=(const AQLDateTime& a) const {return cmp(a) <= 0;}
    // relational operator
    bool                operator>=(const AQLDateTime& a) const {return cmp(a) >= 0;}
    // relational operator
    bool                operator>(const AQLDateTime& a)  const {return cmp(a) > 0;}
    // relational operator
    bool                operator<(const AQLDateTime& a)  const {return cmp(a) < 0;}

protected:
    
    // set the date according to a format
    void				formatWithString(const char_t *st,  const char_t *format);
    
    // get the date according to a format
    void				formatWithLong(AQLString& str, const char_t *format) const;

    // shallow copy of the object
    void				copy(const AQLDateTime& rTime);

private:

    unsigned short     hour_;      // hour(023)
    unsigned short     minute_;    // minute(059)
    unsigned short     second_;    // second(059)
};
