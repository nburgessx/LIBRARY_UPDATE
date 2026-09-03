#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceEventBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADate.h"
#include "LAPricePayOffTool.h"



// LAPriceEventTerminate's function id
#define FN_IR_TRIGGERACTIONTERMINATE		2006
// LAPriceEventTerminate's function name
#define FN_IR_TRIGGERACTIONTERMINATE_STR	"fn_ir_triggeractionterminate"




///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(trade terminate action).
*/
class LAPriceEventTerminate : public LAPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceEventTerminate();
    // destructor	
	virtual ~LAPriceEventTerminate();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;

    // execute trigger action
	virtual void	            doAction(const LADate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<LAPriceEventHolder*>& futureaction,
										 std::vector<LAPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter);

	// set up this class
	virtual	void				setUp(const LADate& basedate,	
									const LAObject& trade,
									LAObject& triggerinfo,
									const LAPricePayOff& payoff,
									bool isCall = false);

	// calculate accrued interest at actiondate
	double						calcAccruedInterest(const LADate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderIter>& iter,
										 unsigned int legno,
										 LAPricePayOffToolHolder& ph) const;
	// calculate notional exchange at actiondate
	double						calcNotionalExchange(const LADate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<LAPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter,
										 unsigned int legno,
										 LAPricePayOffToolHolder& ph) const;

protected:

	bool		mIsAccrual;// consider accrual or not
	DateVector	mLegStart;// leg start date
	BoolVector	mIsArrear;// arrear or advance
	BoolVector	mIsNotionalExchangeAtEnd;// notional exchange at last or not
	
//	std::vector<const LAObject*> mCashlets;// legs

};

