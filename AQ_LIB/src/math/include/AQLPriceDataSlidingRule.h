// AQLPriceDataSlidingRule.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreSystemError.h"
#include "AQLDate.h"
#include "AQLMathCalendar.h"
#include "AQLPriceDataCalendar.h"


// AQLPriceDataSlidingRule Data ID
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
class AQLPriceDataSlidingRule : public AQLPriceDataType
{
public:

	// default constructor
    AQLPriceDataSlidingRule(SlidingRuleType sr = SLIDING_RULE_NO_CHANGE);

	// copy constructor
    AQLPriceDataSlidingRule(const AQLPriceDataSlidingRule& attr);

	// destructor
    virtual ~AQLPriceDataSlidingRule(void);


	// deep copy of the data object
    virtual AQLPriceDataType*	clone() const;

    // return (business day convention) in the string representation
    virtual AQLString	convertToString(void) const;

    // get business day convention
    const SlidingRuleType&	getSlidingRule() const {return mSlidingRuleType;}

    // get the date adjusted the holiday
	AQLDate				getDate(const AQLDate& d,const AQLMathCalendar& c)const;
    
	// get the date adjusted the holiday
	AQLDate				getDate(const AQLDate& d,const AQLPriceDataCalendar& c)const;


    // set business day convention from the specified string format
    virtual void		convertFromString(const AQLString& str);

protected:

    // the contents of the object to be initialized with those of another object
	virtual AQLPriceDataType&	assignment(const AQLPriceDataType& a);

    // compare the contents against the other objecet
	virtual int			compare(const AQLPriceDataType& a) const;

private:

	// check is businessday
	bool isBusinessDay(const AQLDate &d, const AQLPriceDataCalendar& c) const; 

	// business day convention
	SlidingRuleType		mSlidingRuleType; 
};

