#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LARatesAccruedCouponFuncBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"

// Funciton ID of LARatesAggregateCouponCapFloor
#define FN_AGGREGATECOUPONCAPFLOOR	1221
// Function Name of LARatesAggregateCouponCapFloor
#define FN_AGGREGATECOUPONCAPFLOOR_STR	"fn_aggregatecouponcapfloor"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent Coupon Cap function.

	This class derives from LAFunctionBase

*/
class LARatesAggregateCouponCapFloor : public LARatesAccruedCouponFuncBase
{
public:
//  LIFECYCLE
	// constructor
	LARatesAggregateCouponCapFloor();
	// constructor
	explicit LARatesAggregateCouponCapFloor(const DoubleArray& x); 
	// destructor
	virtual ~LARatesAggregateCouponCapFloor();

	//20061017--David--Remove warning:C4512
	LARatesAggregateCouponCapFloor & operator=( const LARatesAggregateCouponCapFloor & ) { return *this; };


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
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
									return LAFunctionBase::operator()(x);								
								}


	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};
	
private:

protected:
	

};
