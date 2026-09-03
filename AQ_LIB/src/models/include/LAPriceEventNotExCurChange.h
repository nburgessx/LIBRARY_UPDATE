#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceEventBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADate.h"
#include "LAPricePayOffTool.h"


// LAPriceEventNotExCurChange's function id
#define FN_IR_TRIGGERACTIONNOTEXCURCHANGE		2014
// LAPriceEventNotExCurChange's function name
#define FN_IR_TRIGGERACTIONNOTEXCURCHANGE_STR	"fn_ir_triggeractionnotexcurchange"



class LAMathFXEntity;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(notional exchange currency change action).
*/
class LAPriceEventNotExCurChange : public LAPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceEventNotExCurChange();
    // destructor	
	virtual ~LAPriceEventNotExCurChange();

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
	
	const LAMathFXEntity*			mpFX;// fx 
	LAString					mCurrency;// currency after changed
	bool						mIsFixingDate;// fixingdate or fixing term				
	LADate						mFixingDate;// fixingdate
	LAString					mFixingTerm;// fixingterm
	const LAPriceDataCalendar*		mpCalendar;// calendar
	const LAPriceDataSlidingRule*	mpSlidingRule;// < slidingrule
	LAFunctionBase*				mpStructureFunc;// < fx rate structure function
			

};


