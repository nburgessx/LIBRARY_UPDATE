#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"

// LAPriceAccruedInterest's function id
#define FN_IR_ACCRUEDINTEREST		2015
// LAPriceAccruedInterest's function name
#define FN_IR_ACCRUEDINTEREST_STR	"fn_ir_accruedinterest"

class LAObject;
class LADataValuation;
class LAPriceDataManager;
class LAPricePayOff;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of accrued interest calculator class.
*/
class LAPriceAccruedInterest : public LACoreFunctionBase
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
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;

    // set up for payoff calculation 
	virtual void	            setUp(const LADate& basedate, LAObject& trade, const LADataValuation& att);
	// calculate payoff
	virtual double				calcAccruedInterest(const LADate& basedate, unsigned int legno) const;

protected:
	
private:
	LAPricePayOff* mpPayOff;// < payoff class
};
