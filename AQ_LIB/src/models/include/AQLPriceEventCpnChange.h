#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceEventBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include "AQLPricePayOffTool.h"



// AQLPriceEventCpnChange's function id
#define FN_IR_TRIGGERACTIONCPNCHANGE		2008
// AQLPriceEventCpnChange's function name
#define FN_IR_TRIGGERACTIONCPNCHANGE_STR	"fn_ir_triggeractioncpnchange"




///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(coupon change action).
*/
class AQLPriceEventCpnChange : public AQLPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceEventCpnChange();
    // destructor	
	virtual ~AQLPriceEventCpnChange();

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

protected:
	std::vector<PayOffToolHolderVector> mPayOff;// payoff
	BoolVector mIsArrearPayment;// arrear payment or advance payment

};


