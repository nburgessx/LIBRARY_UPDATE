/*! @file
@brief Source code of class to represent linear function

This class derives from AQLBlackScholesBaseMethod

*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBlackScholesDigitalFloorletOptionPayoff.h"

using namespace std;


#ifndef PRICING_DATA_DIGITALSPREAD
#define PRICING_DATA_DIGITALSPREAD		"DigitalSpread"					//  data name of DigitalSpread
#endif
//================ AQLBlackShcolesDigitalFloorletOption ===================================


/*!
@brief default constructor
*/
AQLBlackShcolesDigitalFloorletOption::AQLBlackShcolesDigitalFloorletOption()
	: AQLBlackScholesDigitalCapletOption()
{

}


/*!
@brief Make copy(clone) of this class
@return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLBlackShcolesDigitalFloorletOption::clone() const
{
	try
	{
		return new AQLBlackShcolesDigitalFloorletOption(*this);
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
@brief Check function for this class ID
@param[in] id ID to check
@return True or False
*/
bool
AQLBlackShcolesDigitalFloorletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_DIGITALFLOORLETOPTIONFUNC ? true : AQLBlackScholesDigitalCapletOption::isTypeOf(id));
}

/*!
@brief Return this function type
@return function type
*/
function_t
AQLBlackShcolesDigitalFloorletOption::getType() const
{
	return FN_IR_DIGITALFLOORLETOPTIONFUNC;
}


/*!
@brief Return function value
@param[in] x point
@return function value
*/
double
AQLBlackShcolesDigitalFloorletOption::operator()(const DoubleArray& x) const
{
	double digitalCapletValue = AQLBlackScholesDigitalCapletOption::operator()(x);
	double digitalCoupon = getDigitalCoupon();

	return digitalCoupon - digitalCapletValue;
}

