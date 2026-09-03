#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLRatesAccruedCouponFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"

// Funciton ID of AQLRatesAggregateCouponCapFloor
#define FN_AGGREGATECOUPONCAPFLOOR	1221
// Function Name of AQLRatesAggregateCouponCapFloor
#define FN_AGGREGATECOUPONCAPFLOOR_STR	"fn_aggregatecouponcapfloor"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent Coupon Cap function.

	This class derives from AQLFunctionBase

*/
class AQLRatesAggregateCouponCapFloor : public AQLRatesAccruedCouponFuncBase
{
public:
//  LIFECYCLE
	// constructor
	AQLRatesAggregateCouponCapFloor();
	// constructor
	explicit AQLRatesAggregateCouponCapFloor(const DoubleArray& x); 
	// destructor
	virtual ~AQLRatesAggregateCouponCapFloor();

	//20061017--David--Remove warning:C4512
	AQLRatesAggregateCouponCapFloor & operator=( const AQLRatesAggregateCouponCapFloor & ) { return *this; };


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
