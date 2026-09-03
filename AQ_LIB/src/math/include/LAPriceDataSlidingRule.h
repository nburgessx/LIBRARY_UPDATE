// LAPriceDataSlidingRule.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreSystemError.h"
#include "LADate.h"
#include "LAMathCalendar.h"
#include "LAPriceDataCalendar.h"


// LAPriceDataSlidingRule Data ID
#define	DATA_SLIDINGRULE	41

// business day convention rule
enum SlidingRuleType
{
	SLIDING_RULE_NO_CHANGE,		// No change / Unadjusted
	SLIDING_RULE_MOD_FOLLOWING,	// Modified Following Business Day Convention
	SLIDING_RULE_FOLLOWING,		// Following Business Day Convention
	SLIDING_RULE_PRECEDING,		// Preceding Business Day Convention
	SLIDING_RULE_MOD_PRECEDING	// Modified Preceding Business Day Convention
};


/*! 
    @brief Business day convention Data class to represent the data (type business day convention).
*/
class LAPriceDataSlidingRule : public LAPriceDataType
{
public:

	// default constructor
    LAPriceDataSlidingRule(SlidingRuleType sr = SLIDING_RULE_NO_CHANGE);

	// copy constructor
    LAPriceDataSlidingRule(const LAPriceDataSlidingRule& attr);

	// destructor
    virtual ~LAPriceDataSlidingRule(void);


	// deep copy of the data object
    virtual LAPriceDataType*	clone() const;

    // return (business day convention) in the string representation
    virtual LAString	convertToString(void) const;

    // get business day convention
    const SlidingRuleType&	getSlidingRule() const {return mSlidingRuleType;}

    // get the date adjusted the holiday
	LADate				getDate(const LADate& d,const LAMathCalendar& c)const;
    
	// get the date adjusted the holiday
	LADate				getDate(const LADate& d,const LAPriceDataCalendar& c)const;


    // set business day convention from the specified string format
    virtual void		convertFromString(const LAString& str);

protected:

    // the contents of the object to be initialized with those of another object
	virtual LAPriceDataType&	assignment(const LAPriceDataType& a);

    // compare the contents against the other objecet
	virtual int			compare(const LAPriceDataType& a) const;

private:

	// check is businessday
	bool isBusinessDay(const LADate &d, const LAPriceDataCalendar& c) const; 

	// business day convention
	SlidingRuleType		mSlidingRuleType; 
};

