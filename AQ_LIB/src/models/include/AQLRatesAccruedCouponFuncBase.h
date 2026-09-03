#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
 


// Funciton ID of AQLRatesAccruedCouponFuncBase
#define FN_ACCRUEDCOUPONFUNCBASE	1220
// Function Name of AQLRatesAggregateCouponCap
#define FN_ACCRUEDCOUPONFUNCBASE_STR	"fn_accruedcouponfuncbase"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent Coupon Cap function.

	This class derives from AQLFunctionBase

*/
class AQLRatesAccruedCouponFuncBase : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLRatesAccruedCouponFuncBase();
	// destructor
	virtual ~AQLRatesAccruedCouponFuncBase();

	// assignment operator
	AQLRatesAccruedCouponFuncBase & operator=( const AQLRatesAccruedCouponFuncBase & ) { return *this; };


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

