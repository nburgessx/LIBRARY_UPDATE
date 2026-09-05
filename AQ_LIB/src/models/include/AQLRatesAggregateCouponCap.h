#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLRatesAccruedCouponFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
 


// Funciton ID of AQLRatesAggregateCouponCap
#define FN_AGGREGATECOUPONCAP	1221
// Function Name of AQLRatesAggregateCouponCap
#define FN_AGGREGATECOUPONCAP_STR	"fn_aggregatecouponcap"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent Coupon Cap function.

	This class derives from AQLFunctionBase

*/
class AQLRatesAggregateCouponCap : public AQLRatesAccruedCouponFuncBase
{
public:
//  LIFECYCLE
	// constructor
	AQLRatesAggregateCouponCap();
	// constructor
	explicit AQLRatesAggregateCouponCap(const DoubleArray& x); 
	// destructor
	virtual ~AQLRatesAggregateCouponCap();

	// suppress warning C4512
	AQLRatesAggregateCouponCap & operator=( const AQLRatesAggregateCouponCap & ) { return *this; };


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return AQLFunctionBase::operator()(x);								
								}


	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};
	
private:

protected:
	

};
