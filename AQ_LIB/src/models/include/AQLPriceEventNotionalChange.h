#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceEventBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include "AQLPricePayOffTool.h"


// AQLPriceEventNotionalChange's function id
#define FN_IR_TRIGGERACTIONNOTIONALCHANGE		2009
// AQLPriceEventNotionalChange's function name
#define FN_IR_TRIGGERACTIONNOTIONALCHANGE_STR	"fn_ir_triggeractionnotionalchange"




///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of trigger action class(notional change action).
*/
class AQLPriceEventNotionalChange : public AQLPriceEventBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceEventNotionalChange();
    // destructor	
	virtual ~AQLPriceEventNotionalChange();

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
	// get notional array (after change)
	DoubleArray					getNotionalArray(const AQLDate& actiondate, 
												 const PayOffToolHolderVector& payoff, 
												 const PayOffToolHolderIter& iter,
												 bool isarrear,
												 double changeratio,
												 const AQLString& roundfunction,
												 int rounddigit,
												 bool is1stfraction,
												 bool isamountsetting,
												 double amount											 
												 ) const;
	
//	std::vector<PayOffToolHolderVector> mPayOff;// payoff	
	BoolVector mIsArrearPayment;// arrear payment or advance payment
	double mChangeRatio;// notional change ratio
    BoolVector mIsNotionalExchangeAtEnd;// notional exchange at last or not
	BoolVector mIsAmortize;// amoritize or not
	BoolVector mIsAmortizeAmountSetting;// amortize type flag(amortize amount setting or not)
	DoubleArray mAmortizeAmount;// amortize amount(amortize amount setting type)
	AQLStringVector mAmortizeRoundFunction;// round function for amortize notional calculation
	IntArray	mAmortizeRoundDigit;// round digit for amortize notional calculation
	BoolVector mAmortize1stFraction;// amortize fraction flag


};

