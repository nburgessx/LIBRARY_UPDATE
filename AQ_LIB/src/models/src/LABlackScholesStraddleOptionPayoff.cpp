/*! @file
    @brief Source code of class to represent linear function

    This class derives from LABlackScholesBaseMethod

*/
//  2010, Mizuho International London.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackScholesStraddleOption.cpp
//
//  SYNOPSIS    :       LABlackScholesStraddleOption
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


#include "LABlackScholesStraddleOptionPayoff.h"
#include "LABasic.h"
#include "LADist.h"
#include "LAAnalyticFormula.h"
#include "LACoreComponentManager.h"

using namespace std;
//================ LABlackScholesStraddleOption ===================================
/*!
	@brief default constructor
*/
LABlackScholesStraddleOption::LABlackScholesStraddleOption() 
: LABlackScholesCapletOption()
{

}

/*!
	@brief destructor
*/
LABlackScholesStraddleOption::~LABlackScholesStraddleOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LABlackScholesStraddleOption::clone() const
{
    try 
	{
		return new LABlackScholesStraddleOption(*this);
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
LABlackScholesStraddleOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_STRADDLEOPTIONFUNC ? true : LABlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LABlackScholesStraddleOption::getType() const
{
	return FN_IR_STRADDLEOPTIONFUNC;
}


LAStringVector 
LABlackScholesStraddleOption::getOptionTypeVector()
{
	LAStringVector ret(2);
	ret[0] = LAString(CALL);
	ret[1] = LAString(PUT);
	mOptionType = ret;

	return ret;
}


LAStringVector 
LABlackScholesStraddleOption::getBSComponentVector(LAString risktype) const
{
	LAStringVector ret(2);
	ret[0] = LAString(BK) + risktype + LAString(CALL);
	ret[1] = LAString(BK) + risktype + LAString(PUT);

	return ret;
}


LAStringVector 
LABlackScholesStraddleOption::getBSPayoffComponentVector(LAString risktype) const
{
	LAStringVector ret(2);
	ret[0] = LAString(BKPAYOFF) + risktype + LAString(CALL);
	ret[1] = LAString(BKPAYOFF) + risktype + LAString(PUT);
	return ret;
}


/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LABlackScholesStraddleOption::operator()(const DoubleArray& x) const
{
	if (x.size() + 1 != mParam.size() || mParam.size() < 2)
		throw LACoreInvalidData("BSOption size error",__FILE__,__LINE__);

	if (mParam[0] == 0.)
		throw LACoreInvalidData("BSOption coefficient error",__FILE__,__LINE__);
	double gearing = mParam[0];


	unsigned int i = 0; 
	double rate = 0.0;
	for (i = 0; i < x.size(); i++)
	{
		rate += mParam[i] * x[i] / gearing;
	}
//#ifdef ZEROFLOOR
//	rate = LAMath::max(rate, MIN_RATE);
//#endif		
	//option payoff expressed by bying cap and bying floor
	std::vector<double > ret(2);
	for (unsigned int j = 0; j < 2; ++j)
	{
		AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[j]);
		param->F = rate;
		//strikes are same among options
		param->K = mParam[i] / gearing;
		AnalyticBKParam* slideParam(dynamic_cast<AnalyticBKParam*>(param->clone()));
		getForwardShiftParam(slideParam);
		if(slideParam->F > MIN_RATE && slideParam->K > MIN_RATE && !mFixedPayOffs[j])
		{
			ret[j] = mBSAnalyticMethod[j]->calc(*(slideParam)) * LAMath::abs(gearing);
		}
		else
		{
			ret[j] = mBSPayoffMethod[j]->calc(*(slideParam)) * LAMath::abs(gearing);
		}

		// if gearing is negative, option value should be the value converted through put-call parity condition.
		if (gearing < 0.)
		{
			double sgn(0.);
			if (mOptionType[j] == CALL)
				sgn = 1.;
			else if (mOptionType[j] == PUT)
				sgn = -1.;
			else
				throw LACoreInvalidData("Option type error",__FILE__,__LINE__);

		ret[j] = ret[j] - sgn * slideParam->Nu * (slideParam->F - slideParam->K) * LAMath::abs(gearing);
		}

		delete slideParam;
	}
	
	return ret[0] + ret[1];
}


std::vector<AnalyticParam* >
LABlackScholesStraddleOption::getAnalyticParam(void)
{
	mBSInputParam.clear();
		mBSInputParam.push_back(new AnalyticBKParam());
		mBSInputParam.push_back(new AnalyticBKParam());
	
		BoolVector tmp(2,false);
		mFixedPayOffs = tmp;

	return mBSInputParam;
}

void
LABlackScholesStraddleOption::setOptionStrike(void)
{
	if (mParam.empty())
	{
		throw LACoreInvalidData("Coefficient is empty", __FILE__, __LINE__);
	}
	unsigned int paraSize = mBSInputParam.size();
	for (unsigned int i = 0; i < paraSize; i++)
	{
		AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[i]);
		param->K = mParam.back() / mParam.front();
	}
}
