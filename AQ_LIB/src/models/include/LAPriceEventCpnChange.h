#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceEventBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADate.h"
#include "LAPricePayOffTool.h"



// LAPriceEventCpnChange's function id
#define FN_IR_TRIGGERACTIONCPNCHANGE		2008
// LAPriceEventCpnChange's function name
#define FN_IR_TRIGGERACTIONCPNCHANGE_STR	"fn_ir_triggeractioncpnchange"




///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(coupon change action).
*/
class LAPriceEventCpnChange : public LAPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceEventCpnChange();
    // destructor	
	virtual ~LAPriceEventCpnChange();

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

protected:
	std::vector<PayOffToolHolderVector> mPayOff;// payoff
	BoolVector mIsArrearPayment;// arrear payment or advance payment

};


