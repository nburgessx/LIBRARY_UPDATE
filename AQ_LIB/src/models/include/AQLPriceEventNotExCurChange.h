#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceEventBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include "AQLPricePayOffTool.h"


// AQLPriceEventNotExCurChange's function id
#define FN_IR_TRIGGERACTIONNOTEXCURCHANGE		2014
// AQLPriceEventNotExCurChange's function name
#define FN_IR_TRIGGERACTIONNOTEXCURCHANGE_STR	"fn_ir_triggeractionnotexcurchange"



class AQLMathFXEntity;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(notional exchange currency change action).
*/
class AQLPriceEventNotExCurChange : public AQLPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceEventNotExCurChange();
    // destructor	
	virtual ~AQLPriceEventNotExCurChange();

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
	
	const AQLMathFXEntity*			mpFX;// fx 
	AQLString					mCurrency;// currency after changed
	bool						mIsFixingDate;// fixingdate or fixing term				
	AQLDate						mFixingDate;// fixingdate
	AQLString					mFixingTerm;// fixingterm
	const AQLPriceDataCalendar*		mpCalendar;// calendar
	const AQLPriceDataSlidingRule*	mpSlidingRule;// < slidingrule
	AQLFunctionBase*				mpStructureFunc;// < fx rate structure function
			

};


