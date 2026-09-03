#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "LAPricePayOffTool.h"


class AQLObject;
class AQLDate;
class LAPricePayOff;
class LAPriceIndexTool;
class LAPriceIndexToolBase;
class AQLFunctionBase;



///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of tool class of payoff calculation in range accrue.
*/
class LAPricePayOffToolRangeAccrue : public LAPricePayOffTool
{

public:
//  LIFECYCLE
    // default constructor	
	LAPricePayOffToolRangeAccrue();
    // destructor	
	virtual ~LAPricePayOffToolRangeAccrue();
	//	copy constructor
	LAPricePayOffToolRangeAccrue (const LAPricePayOffToolRangeAccrue& v);
	// calculate payoff
	virtual double				calcPayOff(void) const;
	// set up this class
	virtual	void				setUp(const AQLDate& basedate, const AQLObject& trade,
									unsigned int legNo, const AQLObject& cashlet, 
									const LAPricePayOff& payoff,
									unsigned int currentpos);

protected:
	void setUpCalcDays(const AQLDate &start, const AQLDate &end, const DateVector &observationDates, unsigned int &calcDays);
	unsigned int mCalcDays;                  // range accrue calculation days
	AQLDate mRAObservationStart;
	AQLDate mRAObservationEnd;

	class LAPricePayOffToolRangeAccrueImpl
	{
	public:
		// default constructor	
		LAPricePayOffToolRangeAccrueImpl();
		// destructor	
		virtual ~LAPricePayOffToolRangeAccrueImpl();
		//	copy constructor
		LAPricePayOffToolRangeAccrueImpl (const LAPricePayOffToolRangeAccrueImpl& v);
		// clone
		virtual LAPricePayOffToolRangeAccrueImpl* clone(void) const;
		// set up this class
		virtual	void setUp(const AQLDate& basedate, const AQLObject& trade, const LAPricePayOff& payoff, 
						const AQLObject& info, const AQLDate& start, const AQLDate& end, const AQLDate& payment,
						const DateVector* pobservationDates = 0);
		// calc index
		virtual void calcIndex(void);
		// is hit or not
		virtual bool isHit(unsigned int index);

	protected:
		AQLFunctionBase *mpRAFunc;                // range accrue index method
		std::vector<LAPriceIndexTool *> mpRAIndexs; // range accrue index
		mutable double mRAMax;                   // range accrue max
		mutable double mRAMin;                   // range accrue min
		std::vector<std::map<unsigned int, unsigned int> > mRABusDayMap; // range accrue businessday index
		std::vector<LAPriceIndexToolBase *> mpRABIndexs;	// range accrue boundary index
		AQLFunctionBase*	mpRAMaxBFunc;			// range accrue boundary max
		AQLFunctionBase*	mpRAMinBFunc;			// range accrue boundary min
		
	};

	std::vector<LAPricePayOffToolRangeAccrueImpl *> mpRAImpls;	//range accrue implement pointer vector   
	mutable bool mIsAndCondition;	// range accrue multiple index condition is "AND"(TRUE) or "OR"(FALSE)
	mutable bool mIsExcludeAndCondition;	// range accrue include and condition flag
	mutable bool mIsNotCondition;	// range accrue not condition

	AQLFunctionBase* mpHitRateOperator;	// hit rate operator 
	mutable bool mIsHitRateCap;// hit rate cap flag
	mutable bool mIsHitRateFloor;// hit rate floor flag
	mutable double mHitRateCap;// hit rate cap
	mutable double mHitRateFloor;// hit rate floor
	AQLFunctionBase* mpOperator;	// final coupon operator 
	mutable bool mIsRAStrike;// range accrue strike flag
	mutable double mRAStrike;	// range accrue strike
};

