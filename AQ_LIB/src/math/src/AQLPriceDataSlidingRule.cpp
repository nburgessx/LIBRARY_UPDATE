// AQLPriceDataSlidingRule.cpp

#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLPriceDataSlidingRule.h"
#include "ExceptionMacros.h"

using namespace std;

/*!
    @brief default constructor

    @param[in] sr business day convention
*/
AQLPriceDataSlidingRule::AQLPriceDataSlidingRule(SlidingRuleType sr)
		: AQLPriceDataType(DATA_SLIDINGRULE), mSlidingRuleType(sr) 
{
	setNull(false);
}

/*!
    @brief copy constructor

    @param[in] attr original object
*/
AQLPriceDataSlidingRule::AQLPriceDataSlidingRule(const AQLPriceDataSlidingRule& attr)
		: AQLPriceDataType(DATA_SLIDINGRULE), 
		  mSlidingRuleType(attr.mSlidingRuleType)
{
	setNull(attr.isNull());
}

/*!
    @brief destructor
*/
AQLPriceDataSlidingRule::~AQLPriceDataSlidingRule(void) 
{ 
}

/*!
    @brief deep copy of the data object

    @return the copied object
*/
AQLPriceDataType*
AQLPriceDataSlidingRule::clone() const
{
    try {
    	AQLPriceDataSlidingRule* pAttr = new AQLPriceDataSlidingRule(*this);
    	return pAttr;
    }
    catch (bad_alloc & e){
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}

/*!
    @brief business day convention in the string representation
    
    return  business day convention in the string representation

    @retval "MOD_FOLLOWING"  Modified Following Business Day Convention
    @retval "FOLLOWING"  Following Business Day Convention
    @retval "MOD_PRECEDING"  Modified Preceding Business Day Convention
    @retval "PRECEDING"  Preceding Business Day Convention
    @retval "NO_CHANGE"
*/
AQLString
AQLPriceDataSlidingRule::convertToString(void) const
{
	if(isNull()) return NULL_STR;
	
	AQLString	ret;
	switch (mSlidingRuleType)
	{
		case SLIDING_RULE_NO_CHANGE:
			ret = "NO_CHANGE";
			break;
		case SLIDING_RULE_MOD_FOLLOWING:
			ret = "MOD_FOLLOWING";
			break;
		case SLIDING_RULE_FOLLOWING:
			ret = "FOLLOWING";
			break;
		case SLIDING_RULE_PRECEDING:
			ret = "PRECEDING";
			break;
		case SLIDING_RULE_MOD_PRECEDING:
			ret = "MOD_PRECEDING";
			break;
		default:
			AQ_THROW("Invalid Business Day Adjustment")
			break;
	}
	return ret;
}

/*!
    @brief get the date adjusted the holiday
    
    if d is not a holiday, return d

    @param[in] d the date not adjusted the holiday
    @param[in] c the holiday calendar

    @return the date adjusted the holiday
*/
AQLDate AQLPriceDataSlidingRule::getDate(const AQLDate& d, const AQLCalendar& c)const
{       
	AQLDate	ret = d;
	if (c.isHoliday(ret))
	{
		switch(mSlidingRuleType)
		{
		case SLIDING_RULE_MOD_FOLLOWING:
			ret.addDays(1);
			while(c.isHoliday(ret))
			{
				ret.addDays(1);
			}
			// change month ?
			if (ret.monthOfYear() != d.monthOfYear())
			{
				ret = d;
				ret.addDays(-1);
				while(c.isHoliday(ret))
				{
					ret.addDays(-1);
				}
			}
			break;
		case SLIDING_RULE_FOLLOWING:
			ret.addDays(1);
			while(c.isHoliday(ret))
			{
				ret.addDays(1);
			}
			break;
		case SLIDING_RULE_PRECEDING: 
			ret.addDays(-1);
			while(c.isHoliday(ret))
			{
				ret.addDays(-1);
			}
			break;
		case SLIDING_RULE_MOD_PRECEDING: 
			ret.addDays(-1);
			while(c.isHoliday(ret))
			{
				ret.addDays(-1);
			}
			// change month ?
			if (ret.monthOfYear() != d.monthOfYear())
			{
				ret = d;
				ret.addDays(1);
				while(c.isHoliday(ret))
				{
					ret.addDays(1);
				}
			}
			break;
		case SLIDING_RULE_NO_CHANGE: //no change
			break;
		}
	}
	return ret;
}

/*!
    @brief get the date adjusted the holiday adjusted
    
     if d is not a holiday, return d

    @param[in] d the date not adjusted the holiday
    @param[in] c the holiday calendar Data

    @return the date adjusted the holiday
*/
AQLDate AQLPriceDataSlidingRule::getDate(const AQLDate& d, const AQLPriceDataCalendar& c)const
{
	// Return Input if using Unadjusted or is already a Good Business Day
	if( mSlidingRuleType == SLIDING_RULE_NO_CHANGE || isBusinessDay( d, c ) )
	{
		return d;
	}
	
	AQLDate result = d;
	bool isHoliday = false;

	switch(mSlidingRuleType)
	{
		case SLIDING_RULE_MOD_FOLLOWING:
		{
			result.addDays(1);
			while(!isBusinessDay(result, c))
			{
				result.addDays(1);
			}
			// change month ?
			if (result.monthOfYear() != d.monthOfYear())
			{
				result = d;
				result.addDays(-1);
				while(!isBusinessDay(result, c))
				{
					result.addDays(-1);
				}
			}
			break;
		}
		case SLIDING_RULE_FOLLOWING:
		{
			result.addDays(1);
			while(!isBusinessDay(result, c))
			{
				result.addDays(1);
			}
			break;
		}
		case SLIDING_RULE_PRECEDING:
		{
			result.addDays(-1);
			while(!isBusinessDay(result, c))
			{
				result.addDays(-1);
			}
			break;
		}
		case SLIDING_RULE_MOD_PRECEDING:
		{
			result.addDays(-1);
			while(!isBusinessDay(result, c))
			{
				result.addDays(-1);
			}
			// change month ?
			if (result.monthOfYear() != d.monthOfYear())
			{
				result = d;
				result.addDays(1);
				while(!isBusinessDay(result, c))
				{
					result.addDays(1);
				}
			}
			break;
		}
		default: 
		{
			// No Change
			break;
		}
	}
	return result;
}

/*!
    @brief set the business day convention information from a string of business day convention that you specify

    Argument to specify one of the following character strings <br>
	 1. "MOD_FOLLOWING"  Modified Following Business Day Convention <br>
	 2. "FOLLOWING"      Following Business Day Convention <br>
     3. "MOD_PRECEDING"  Modified Preceding Business Day Convention <br>
     4. "PRECEDING"      Preceding Business Day Convention <br>
     5. "NO_CHANGE"      No change

    @param[in] str string representing business day convention
*/
void
AQLPriceDataSlidingRule::convertFromString(const AQLString& str)
{

	AQLString slidingRuleStr;
	bool ret = strToData( str, slidingRuleStr );
	if ( ret || slidingRuleStr.size() == 0 ) 
	{
		// notify of the change
		update();
		setNull(true);
	}
	else
	{
		setNull(false);
		slidingRuleStr.toUpper();

		SlidingRuleType origType = mSlidingRuleType;
		if ( slidingRuleStr == "MOD_FOLLOWING" || slidingRuleStr == "MF" || slidingRuleStr == "MOD_FOL" )
		{
			mSlidingRuleType = SLIDING_RULE_MOD_FOLLOWING;
		}
		else if ( slidingRuleStr == "FOLLOWING" || slidingRuleStr == "F" || slidingRuleStr == "FOL" )
		{
			mSlidingRuleType = SLIDING_RULE_FOLLOWING;
		}
		else if ( slidingRuleStr == "PRECEDING" || slidingRuleStr == "P" || slidingRuleStr == "PRE" )
		{
			mSlidingRuleType = SLIDING_RULE_PRECEDING;
		}
		else if ( slidingRuleStr == "MOD_PRECEDING" || slidingRuleStr == "MP" || slidingRuleStr == "MOD_PRE" )
		{
			mSlidingRuleType = SLIDING_RULE_MOD_PRECEDING;
		}
		else if ( slidingRuleStr == "NO_CHANGE" || slidingRuleStr == "U" || slidingRuleStr == "UNADJUSTED" || slidingRuleStr == "NO_CHG" )
		{
			mSlidingRuleType = SLIDING_RULE_NO_CHANGE;
		}
		else
		{
			setNull();
			AQ_THROW( "Invalid BusinessDayConvention: '" + slidingRuleStr + "', must be Following (F), Mod_Following (MF), Preceding (P), Mod_Preceding (MP) or Unadjusted (U)." )
		}

		if (origType != mSlidingRuleType)
		{
			update();
		}
	}

}

/*!
    @brief the contents of the object to be initialized with those of another object

    @param[in] a another object initialized with this contents
*/
AQLPriceDataType&
AQLPriceDataSlidingRule::assignment(const AQLPriceDataType& a)
{
	if (this == &a ) return *this;
	if (a.getType() != DATA_SLIDINGRULE)
	{
		// exception
		AQLString	msg = "Assignment error for AQLPriceDataSlidingRule : from ";
		msg += AQLString(a.getType());
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	update();
	AQLPriceDataSlidingRule b = dynamic_cast<const AQLPriceDataSlidingRule&>(a);
	mSlidingRuleType = b.mSlidingRuleType;
	setNull(a.isNull());
	return *this;	
}

/*!
    @brief compare the contents against the other objecet
    
    an exception occurs when different compared to the Data object

    @param[in] a the object to be compared
  
	@return (type of business day convention that this object has) - (type of business day convention that "a" has)
*/
int
AQLPriceDataSlidingRule::compare(const AQLPriceDataType& a) const
{
	if (a.getType() != DATA_SLIDINGRULE)
	{
		// exception
		AQLString	msg = "Compare error for AQLPriceDataSlidingRule : from ";
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
	AQLPriceDataSlidingRule	b = dynamic_cast<const AQLPriceDataSlidingRule&>(a);
	return mSlidingRuleType - b.mSlidingRuleType;
}

/*!
    @brief check is businessday
    
    @param[in] date
	@param[in] calendars 
  
	@return bool
*/
bool
AQLPriceDataSlidingRule::isBusinessDay(const AQLDate &d, const AQLPriceDataCalendar& c) const
{
	const set<const AQLCalendar *> &calSet = c.getCalendarSet();
	set<const AQLCalendar *>::const_iterator it = calSet.begin();

	while (it != calSet.end())
	{
		if ((*it)->isHoliday(d))
		{
			return false;
		}
		++it;
	}

	return true;
}
