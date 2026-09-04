/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLBlackScholesBaseMethod
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBlackScholesCapletSpreadOptionPayoff.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLAnalyticFormula.h"
#include "AQLCoreComponentManager.h"

using namespace std;
//================ AQLBlackScholesCapletSpreadOption ===================================
/*!
	@brief default constructor
*/
AQLBlackScholesCapletSpreadOption::AQLBlackScholesCapletSpreadOption() 
: AQLBlackScholesCapletOption()
{

}

/*!
	@brief destructor
*/
AQLBlackScholesCapletSpreadOption::~AQLBlackScholesCapletSpreadOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLBlackScholesCapletSpreadOption::clone() const
{
    try 
	{
		return new AQLBlackScholesCapletSpreadOption(*this);
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
AQLBlackScholesCapletSpreadOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_CAPLETSPREADOPTIONFUNC ? true : AQLBlackScholesCapletOption::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLBlackScholesCapletSpreadOption::getType() const
{
	return FN_IR_CAPLETSPREADOPTIONFUNC;
}


AQLStringVector
AQLBlackScholesCapletSpreadOption::getOptionTypeVector()
{
	AQLStringVector ret(2);
	ret[0] = AQLString(CALL);
	ret[1] = AQLString(CALL);
	mOptionType = ret;

	return ret;
}


AQLStringVector 
AQLBlackScholesCapletSpreadOption::getBSComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2);
	ret[0] = AQLString(BK) + risktype + AQLString(CALL);
	ret[1] = AQLString(BK) + risktype + AQLString(CALL);

	return ret;

}


AQLStringVector 
AQLBlackScholesCapletSpreadOption::getBSPayoffComponentVector(AQLString risktype) const
{
	AQLStringVector ret(2);
	ret[0] = AQLString(BKPAYOFF) + risktype + AQLString(CALL);
	ret[1] = AQLString(BKPAYOFF) + risktype + AQLString(CALL);

	return ret;

}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLBlackScholesCapletSpreadOption::operator()(const DoubleArray& x) const
{
	if (x.size() + 2 != mParam.size() || mParam.size() < 2)
		throw AQLCoreInvalidData("BSOption size error",__FILE__,__LINE__);

	if (mParam[0] == 0.)
		throw AQLCoreInvalidData("BSOption coefficient error",__FILE__,__LINE__);
	double gearing = mParam[0];

	
	unsigned int i = 0; 
	double rate = 0.0;
	for (i = 0; i < x.size(); i++)
	{
		rate += mParam[i] * x[i] / gearing;
	}
//#ifdef ZEROFLOOR
//	rate = AQLMath::max(rate, MIN_RATE);
//#endif	
	//option payoff expressed by bying cap1 and selling cap2
	std::vector<double > ret(2);
	for (unsigned int j = 0; j < 2; ++j)
	{
		AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[j]);
		param->F = rate;
		//strikes are diffrent between options
		param->K = mParam[i + j] / gearing;
		AnalyticBKParam* slideParam(dynamic_cast<AnalyticBKParam*>(param->clone()));
		getForwardShiftParam(slideParam);
		if(slideParam->F > MIN_RATE && slideParam->K > MIN_RATE && !mFixedPayOffs[j])
		{
			ret[j] = mBSAnalyticMethod[j]->calc(*(slideParam)) * AQLMath::abs(gearing);
		}
		else
		{
			ret[j] = mBSPayoffMethod[j]->calc(*(slideParam)) * AQLMath::abs(gearing);
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
				throw AQLCoreInvalidData("Option type error",__FILE__,__LINE__);

		ret[j] = ret[j] - sgn * slideParam->Nu * (slideParam->F - slideParam->K) * AQLMath::abs(gearing);
		}

		delete slideParam;
	}

	return ret[0] - ret[1];
}


std::vector<AnalyticParam* >
AQLBlackScholesCapletSpreadOption::getAnalyticParam(void)
{
	if (mBSInputParam.size() == 0 || mBSInputParam[0] == NULL)
	{
		mBSInputParam.push_back(new AnalyticBKParam());
		mBSInputParam.push_back(new AnalyticBKParam());

		BoolVector tmp(2,false);
		mFixedPayOffs = tmp;

	}
	
	return mBSInputParam;
}

