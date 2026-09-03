/*! @file
    @brief Source code of class to represent linear function

    This class derives from LABlackScholesBaseMethod

*/
//  2010, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackScholesDigitalCapletOption.cpp
//
//  SYNOPSIS    :       LABlackScholesDigitalCapletOption
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


#include "LABlackScholesDigitalCapletOptionPayoff.h"

using namespace std;

// DEFINE //
#ifndef PRICING_DATA_DIGITALSPREAD
#define PRICING_DATA_DIGITALSPREAD		"DigitalSpread"					//  data name of DigitalSpread
#endif
//================ LABlackScholesDigitalCapletOption ===================================


/*!
@brief default constructor
*/
LABlackScholesDigitalCapletOption::LABlackScholesDigitalCapletOption()
	: LABlackScholesCapletOption(), mCallSpreadValue(0.), mDigitalCoupon(0.)
{

}


/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LABlackScholesDigitalCapletOption::clone() const
{
    try 
	{
		return new LABlackScholesDigitalCapletOption(*this);
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
LABlackScholesDigitalCapletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_DIGITALCAPLETOPTIONFUNC ? true : LABlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LABlackScholesDigitalCapletOption::getType() const
{
	return FN_IR_DIGITALCAPLETOPTIONFUNC;
}


/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LABlackScholesDigitalCapletOption::operator()(const DoubleArray& x) const
{
	double callSpreadValue = getCallSpreadValue();

	LABlackScholesDigitalCapletOption capLow(*this);
	capLow.setParam(DoubleArray{ mParam[0], mParam[1] - 0.5 * callSpreadValue });
	double resLow = capLow.LABlackScholesCapletOption::operator()(x, 0);

	LABlackScholesDigitalCapletOption capHigh(*this);
	capHigh.setParam(DoubleArray{ mParam[0], mParam[1] + 0.5 * callSpreadValue });
	double resHigh = capHigh.LABlackScholesCapletOption::operator()(x, 1);

	return (resLow - resHigh) * mDigitalCoupon / callSpreadValue;
}

// return call spread value
double 
LABlackScholesDigitalCapletOption::getCallSpreadValue() const
{
	return mCallSpreadValue;
}

// set call spread value and call spread option strike
void
LABlackScholesDigitalCapletOption::setCallSpreadValue(double callSpreadValue)
{
	mCallSpreadValue = callSpreadValue;
	setOptionStrike();
}

// return digital coupon value
double	
LABlackScholesDigitalCapletOption::getDigitalCoupon() const
{
	return mDigitalCoupon;
}

// set digital coupon value
void
LABlackScholesDigitalCapletOption::setDigitalCoupon(double digitalCoupon)
{
	mDigitalCoupon = digitalCoupon;
}

// returns option type (two call options for call spread eavaluation)
LAStringVector
LABlackScholesDigitalCapletOption::getOptionTypeVector()
{
	LAStringVector ret(2, LAString(CALL));
	mOptionType = ret;
	return ret;
}

// return string representation of analytic method
LAStringVector
LABlackScholesDigitalCapletOption::getBSComponentVector(LAString risktype) const
{
	LAStringVector ret(2, LAString(BK) + risktype + LAString(CALL));
	return ret;
}

// returns string representation of payoff method
LAStringVector
LABlackScholesDigitalCapletOption::getBSPayoffComponentVector(LAString risktype) const
{
	LAStringVector ret(2, LAString(BKPAYOFF) + risktype + LAString(CALL));
	return ret;
}


/*!
	@ return:  analytic formula used for evaluation
	@ side effect: set value of mBSInputParam and mFixedPayOffs
	@ note: owership of AnalyticBKParam is passed to the caller
*/
vector<AnalyticParam* >
LABlackScholesDigitalCapletOption::getAnalyticParam(void)
{
	mBSInputParam = { new AnalyticBKParam(), new AnalyticBKParam() };
	mFixedPayOffs = BoolVector(2, false);

	return mBSInputParam;
}

// set option stike
void
LABlackScholesDigitalCapletOption::setOptionStrike(void)
{
	double callSpreadValue = getCallSpreadValue();
	mBSInputParam[0]->K = mParam[1] / mParam[0] - 0.5 * callSpreadValue;
	mBSInputParam[1]->K = mParam[1] / mParam[0] + 0.5 * callSpreadValue;
	
}
