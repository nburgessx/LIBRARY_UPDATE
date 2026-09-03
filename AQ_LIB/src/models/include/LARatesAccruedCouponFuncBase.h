#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
 


// Funciton ID of LARatesAccruedCouponFuncBase
#define FN_ACCRUEDCOUPONFUNCBASE	1220
// Function Name of LARatesAggregateCouponCap
#define FN_ACCRUEDCOUPONFUNCBASE_STR	"fn_accruedcouponfuncbase"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent Coupon Cap function.

	This class derives from LAFunctionBase

*/
class LARatesAccruedCouponFuncBase : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LARatesAccruedCouponFuncBase();
	// destructor
	virtual ~LARatesAccruedCouponFuncBase();

	// assignment operator
	LARatesAccruedCouponFuncBase & operator=( const LARatesAccruedCouponFuncBase & ) { return *this; };


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	
	
private:

protected:
	

};

