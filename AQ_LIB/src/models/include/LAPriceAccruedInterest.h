#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"

// LAPriceAccruedInterest's function id
#define FN_IR_ACCRUEDINTEREST		2015
// LAPriceAccruedInterest's function name
#define FN_IR_ACCRUEDINTEREST_STR	"fn_ir_accruedinterest"

class AQLObject;
class AQLDataValuation;
class AQLPriceDataManager;
class LAPricePayOff;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of accrued interest calculator class.
*/
class LAPriceAccruedInterest : public AQLCoreFunctionBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceAccruedInterest();
    // destructor	
	virtual ~LAPriceAccruedInterest();
	// copy constructor
	LAPriceAccruedInterest(const LAPriceAccruedInterest& v);
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;

    // set up for payoff calculation 
	virtual void	            setUp(const AQLDate& basedate, AQLObject& trade, const AQLDataValuation& att);
	// calculate payoff
	virtual double				calcAccruedInterest(const AQLDate& basedate, unsigned int legno) const;

protected:
	
private:
	LAPricePayOff* mpPayOff;// < payoff class
};
