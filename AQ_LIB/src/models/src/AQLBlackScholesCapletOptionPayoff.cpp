/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLBlackScholesBaseMethod

*/
//  2010, AlgoQuantHub.

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBlackScholesCapletOptionPayoff.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLAnalyticFormula.h"
#include "AQLCoreComponentManager.h"

using namespace std;
//================ AQLBlackScholesCapletOption ===================================
/*!
	@brief default constructor
*/
AQLBlackScholesCapletOption::AQLBlackScholesCapletOption() 
: AQLBlackScholesBaseMethod()
{

}

/*!
	@brief destructor
*/
AQLBlackScholesCapletOption::~AQLBlackScholesCapletOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLBlackScholesCapletOption::clone() const
{
    try 
	{
		return new AQLBlackScholesCapletOption(*this);
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
AQLBlackScholesCapletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_CAPLETOPTIONFUNC ? true : AQLBlackScholesBaseMethod::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLBlackScholesCapletOption::getType() const
{
	return FN_IR_CAPLETOPTIONFUNC;
}

/*!
	@brief Return function value
	@param[in] x point
	@return function value
*/
double
AQLBlackScholesCapletOption::operator()(const DoubleArray& x) const
{
	return operator()(x, 0);
}

/*!
    @brief Return function value
	@param[in] x: point
	@param[in] calcIndex: The index of mBSAnalyticMethod to be calculated. 
						This function is used by, for example, the evaluation of digital caplet using call spread. 
	@return function value
*/
double
AQLBlackScholesCapletOption::operator()(const DoubleArray& x, size_t calcIndex) const
{
	if (x.size() + 1 != mParam.size() || mParam.size() < 2)
		throw AQLCoreInvalidData("BSOption size error",__FILE__,__LINE__);

	//if (mParam[0] <= 0.)
	//	throw AQLCoreInvalidData("BSOption coefficient error",__FILE__,__LINE__);
	
	//calc gearing
	double gearing = 0.;
	for (unsigned int j = 0; j < x.size(); ++j)
	{
		gearing += mParam[j];
	}

	if (gearing == 0.)
		throw AQLCoreInvalidData("BSOption coefficient error",__FILE__,__LINE__);

	AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[calcIndex]);
	unsigned int i = 0; 
	double rate = 0.0;
	for (i = 0; i < x.size(); i++)
	{
		rate += mParam[i] * x[i] / gearing;
	}
//#ifdef ZEROFLOOR
//	rate = AQLMath::max(rate, MIN_RATE);
//#endif
	param->F = rate;
	param->K = mParam[i] / gearing;
	AnalyticBKParam* slideParam(dynamic_cast<AnalyticBKParam*>(param->clone()));
	getForwardShiftParam(slideParam);

	double ret(0.);
	if (slideParam->F > MIN_RATE && slideParam->K > MIN_RATE && !mFixedPayOffs[calcIndex])
	{
		ret = mBSAnalyticMethod[calcIndex]->calc(*(slideParam)) * AQLMath::abs(gearing);
		ret += calcConvexityAdjustment(*slideParam, ret / AQLMath::abs(gearing)) * AQLMath::abs(gearing);
	}
	else
	{
		ret = mBSPayoffMethod[calcIndex]->calc(*(slideParam)) * AQLMath::abs(gearing);
	}

	// if gearing is negative, option value should be the value converted through put-call parity condition.
	if (gearing < 0.)
	{
		double sgn(0.);
		if (mOptionType[calcIndex] == CALL)
			sgn = 1.;
		else if (mOptionType[calcIndex] == PUT)
			sgn = -1.;
		else
			throw AQLCoreInvalidData("Option type error",__FILE__,__LINE__);

		ret = ret - sgn * slideParam->Nu * (slideParam->F - slideParam->K) * AQLMath::abs(gearing);
	}
	
	delete slideParam;

	return ret;
}

AQLStringVector
AQLBlackScholesCapletOption::getOptionTypeVector()
{
	AQLStringVector ret(1);
	ret[0] = AQLString(CALL);
	mOptionType = ret;
	return ret;
}

AQLStringVector 
AQLBlackScholesCapletOption::getBSComponentVector(AQLString risktype) const
{
	AQLStringVector ret(1);
	ret[0] = AQLString(BK) + risktype + AQLString(CALL);
	return ret;
}

AQLStringVector 
AQLBlackScholesCapletOption::getBSPayoffComponentVector(AQLString risktype) const
{
	AQLStringVector ret(1);
	ret[0] = AQLString(BKPAYOFF) + risktype + AQLString(CALL);
	return ret;
}

std::vector<AnalyticParam* >
AQLBlackScholesCapletOption::getAnalyticParam(void)
{
		mBSInputParam.clear();
		mBSInputParam.push_back(new AnalyticBKParam());

		BoolVector tmp(1,false);
		mFixedPayOffs = tmp;

	
	return mBSInputParam;
}

void 
AQLBlackScholesCapletOption::setOptionTerm(double Te)
{
	//if size > 1 then collar or strangle ..etc
	for (unsigned int i = 0; i < mBSInputParam.size(); i++)
	{
		AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[i]);
		param->Te = Te;
	}
	return;
}

void 
AQLBlackScholesCapletOption::setOptionActualTerm(double actT)
{
	//if size > 1 then collar or strangle ..etc
	for (unsigned int i = 0; i < mBSInputParam.size(); i++)
	{
		AnalyticParam* param = mBSInputParam[i];
		param->actT = actT;
	}
	return;
}

void 
AQLBlackScholesCapletOption::setOptionNumeraire(void)
{
	//if size > 1 then collar or strangle ..etc
	for (unsigned int i = 0; i < mBSInputParam.size(); i++)
	{
		AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[i]);
		param->Nu = 1.0;
	}
}

void
AQLBlackScholesCapletOption::setOptionStrike(void)
{
	//if size > 1 then collar or strangle ..etc
	unsigned int coeffsize = mParam.size();
	{
		unsigned int paraSize = mBSInputParam.size();
		for (unsigned int i = 0; i < paraSize; i++)
		{
			if (coeffsize-1-i < 0 || mParam[0] == 0.)
				throw AQLCoreInvalidData("Coefficient Error",__FILE__,__LINE__);

			AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[paraSize-1-i]);
			param->K = mParam[coeffsize-1-i] / mParam[0];
		}
		
	}
}

void
AQLBlackScholesCapletOption::setIsAfterMaturity(bool isaftermaturity)
{
	if (!isaftermaturity)
		return;

	for (unsigned int i = 0; i < mBSInputParam.size(); i++)
		mFixedPayOffs[i] = true;

}


