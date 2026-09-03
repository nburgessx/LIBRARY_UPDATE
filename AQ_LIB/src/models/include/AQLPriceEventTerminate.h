#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceEventBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include "AQLPricePayOffTool.h"



// AQLPriceEventTerminate's function id
#define FN_IR_TRIGGERACTIONTERMINATE		2006
// AQLPriceEventTerminate's function name
#define FN_IR_TRIGGERACTIONTERMINATE_STR	"fn_ir_triggeractionterminate"




///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(trade terminate action).
*/
class AQLPriceEventTerminate : public AQLPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceEventTerminate();
    // destructor	
	virtual ~AQLPriceEventTerminate();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;

    // execute trigger action
	virtual void	            doAction(const AQLDate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<AQLPriceEventHolder*>& futureaction,
										 std::vector<AQLPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter);

	// set up this class
	virtual	void				setUp(const AQLDate& basedate,	
									const AQLObject& trade,
									AQLObject& triggerinfo,
									const AQLPricePayOff& payoff,
									bool isCall = false);

	// calculate accrued interest at actiondate
	double						calcAccruedInterest(const AQLDate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderIter>& iter,
										 unsigned int legno,
										 AQLPricePayOffToolHolder& ph) const;
	// calculate notional exchange at actiondate
	double						calcNotionalExchange(const AQLDate& actiondate,
										 double actiontime,
										 std::vector<PayOffToolHolderVector>& payoff,
										 std::vector<PayOffToolHolderVector>& extrapayoff,
										 std::vector<AQLPriceEventHolder*>& pastaction,
										 std::vector<PayOffToolHolderIter>& iter,
										 unsigned int legno,
										 AQLPricePayOffToolHolder& ph) const;

protected:

	bool		mIsAccrual;// consider accrual or not
	DateVector	mLegStart;// leg start date
	BoolVector	mIsArrear;// arrear or advance
	BoolVector	mIsNotionalExchangeAtEnd;// notional exchange at last or not
	
//	std::vector<const AQLObject*> mCashlets;// legs

};

