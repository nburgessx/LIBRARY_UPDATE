/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLBlackScholesBaseMethod

*/
//  2010, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLBlackScholesDigitalCapletOption.cpp
//
//  SYNOPSIS    :       AQLBlackScholesDigitalCapletOption
//  DESCRIPTION :       Source code of class to represent function led by AQLBlackScholesBase class
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


#include "AQLBlackScholesDigitalCapletOptionPayoff.h"

using namespace std;

// DEFINE //
#ifndef PRICING_DATA_DIGITALSPREAD
#define PRICING_DATA_DIGITALSPREAD		"DigitalSpread"					//  data name of DigitalSpread
#endif
//================ AQLBlackScholesDigitalCapletOption ===================================


/*!
@brief default constructor
*/
AQLBlackScholesDigitalCapletOption::AQLBlackScholesDigitalCapletOption()
	: AQLBlackScholesCapletOption(), mCallSpreadValue(0.), mDigitalCoupon(0.)
{

}


/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLBlackScholesDigitalCapletOption::clone() const
{
    try 
	{
		return new AQLBlackScholesDigitalCapletOption(*this);
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
AQLBlackScholesDigitalCapletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_DIGITALCAPLETOPTIONFUNC ? true : AQLBlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLBlackScholesDigitalCapletOption::getType() const
{
	return FN_IR_DIGITALCAPLETOPTIONFUNC;
}


/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLBlackScholesDigitalCapletOption::operator()(const DoubleArray& x) const
{
	double callSpreadValue = getCallSpreadValue();

	AQLBlackScholesDigitalCapletOption capLow(*this);
	capLow.setParam(DoubleArray{ mParam[0], mParam[1] - 0.5 * callSpreadValue });
	double resLow = capLow.AQLBlackScholesCapletOption::operator()(x, 0);

	AQLBlackScholesDigitalCapletOption capHigh(*this);
	capHigh.setParam(DoubleArray{ mParam[0], mParam[1] + 0.5 * callSpreadValue });
	double resHigh = capHigh.AQLBlackScholesCapletOption::operator()(x, 1);

	return (resLow - resHigh) * mDigitalCoupon / callSpreadValue;
}

// return call spread value
double 
AQLBlackScholesDigitalCapletOption::getCallSpreadValue() const
{
	return mCallSpreadValue;
}

// set call spread value and call spread option strike
void
AQLBlackScholesDigitalCapletOption::setCallSpreadValue(double callSpreadValue)
{
	mCallSpreadValue = callSpreadValue;
	setOptionStrike();
}

// return digital coupon value
double	
AQLBlackScholesDigitalCapletOption::getDigitalCoupon() const
{
	return mDigitalCoupon;
}

// set digital coupon value
void
AQLBlackScholesDigitalCapletOption::setDigitalCoupon(double digitalCoupon)
{
	mDigitalCoupon = digitalCoupon;
}

// returns option type (two call options for call spread eavaluation)
AQLStringVector
AQLBlackScholesDigitalCapletOption::getOptionTypeVector()
{
	AQLStringVector ret(2, AQLString(CALL));
	mOptionType = ret;
	return ret;
}

// return string representation of analytic method
AQLStringVector
AQLBlackScholesDigitalCapletOption::getBSComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2, AQLString(BK) + risktype + AQLString(CALL));
	return ret;
}

// returns string representation of payoff method
AQLStringVector
AQLBlackScholesDigitalCapletOption::getBSPayoffComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2, AQLString(BKPAYOFF) + risktype + AQLString(CALL));
	return ret;
}


/*!
	@ return:  analytic formula used for evaluation
	@ side effect: set value of mBSInputParam and mFixedPayOffs
	@ note: owership of AnalyticBKParam is passed to the caller
*/
vector<AnalyticParam* >
AQLBlackScholesDigitalCapletOption::getAnalyticParam(void)
{
	mBSInputParam = { new AnalyticBKParam(), new AnalyticBKParam() };
	mFixedPayOffs = BoolVector(2, false);

	return mBSInputParam;
}

// set option stike
void
AQLBlackScholesDigitalCapletOption::setOptionStrike(void)
{
	double callSpreadValue = getCallSpreadValue();
	mBSInputParam[0]->K = mParam[1] / mParam[0] - 0.5 * callSpreadValue;
	mBSInputParam[1]->K = mParam[1] / mParam[0] + 0.5 * callSpreadValue;
	
}
