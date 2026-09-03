/*! @file
@brief Source code of class to represent linear function

This class derives from LABlackScholesBaseMethod

*/
//  2010, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackShcolesDigitalFloorletOption.cpp
//
//  SYNOPSIS    :       LABlackShcolesDigitalFloorletOption
//  DESCRIPTION :       Source code of class to represent function led by LABlackScholesBase class
//                      
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LABlackScholesDigitalFloorletOptionPayoff.h"

using namespace std;


#ifndef PRICING_DATA_DIGITALSPREAD
#define PRICING_DATA_DIGITALSPREAD		"DigitalSpread"					//  data name of DigitalSpread
#endif
//================ LABlackShcolesDigitalFloorletOption ===================================


/*!
@brief default constructor
*/
LABlackShcolesDigitalFloorletOption::LABlackShcolesDigitalFloorletOption()
	: LABlackScholesDigitalCapletOption()
{

}


/*!
@brief Make copy(clone) of this class
@return Deep copy of this class
*/
LACoreFunctionBase*
LABlackShcolesDigitalFloorletOption::clone() const
{
	try
	{
		return new LABlackShcolesDigitalFloorletOption(*this);
	}
	catch (bad_alloc & e)
	{
		throw LACoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
@brief Check function for this class ID
@param[in] id ID to check
@return True or False
*/
bool
LABlackShcolesDigitalFloorletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_DIGITALFLOORLETOPTIONFUNC ? true : LABlackScholesDigitalCapletOption::isTypeOf(id));
}

/*!
@brief Return this function type
@return function type
*/
function_t
LABlackShcolesDigitalFloorletOption::getType() const
{
	return FN_IR_DIGITALFLOORLETOPTIONFUNC;
}


/*!
@brief Return function value
@param[in] x point
@return function value
*/
double
LABlackShcolesDigitalFloorletOption::operator()(const DoubleArray& x) const
{
	double digitalCapletValue = LABlackScholesDigitalCapletOption::operator()(x);
	double digitalCoupon = getDigitalCoupon();

	return digitalCoupon - digitalCapletValue;
}

