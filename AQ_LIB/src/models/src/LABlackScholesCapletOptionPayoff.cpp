/*! @file
    @brief Source code of class to represent linear function

    This class derives from LABlackScholesBaseMethod

*/
//  2010, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackScholesCapletOption.cpp
//
//  SYNOPSIS    :       LABlackScholesCapletOption
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


#include "LABlackScholesCapletOptionPayoff.h"
#include "LABasic.h"
#include "LADist.h"
#include "LAAnalyticFormula.h"
#include "LACoreComponentManager.h"

using namespace std;
//================ LABlackScholesCapletOption ===================================
/*!
	@brief default constructor
*/
LABlackScholesCapletOption::LABlackScholesCapletOption() 
: LABlackScholesBaseMethod()
{

}

/*!
	@brief destructor
*/
LABlackScholesCapletOption::~LABlackScholesCapletOption() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LABlackScholesCapletOption::clone() const
{
    try 
	{
		return new LABlackScholesCapletOption(*this);
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
LABlackScholesCapletOption::isTypeOf(function_t id) const
{
	return (id == FN_IR_CAPLETOPTIONFUNC ? true : LABlackScholesBaseMethod::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LABlackScholesCapletOption::getType() const
{
	return FN_IR_CAPLETOPTIONFUNC;
}

/*!
	@brief Return function value
	@param[in] x point
	@return function value
*/
double
LABlackScholesCapletOption::operator()(const DoubleArray& x) const
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
LABlackScholesCapletOption::operator()(const DoubleArray& x, size_t calcIndex) const
{
	if (x.size() + 1 != mParam.size() || mParam.size() < 2)
		throw LACoreInvalidData("BSOption size error",__FILE__,__LINE__);

	//if (mParam[0] <= 0.)
	//	throw LACoreInvalidData("BSOption coefficient error",__FILE__,__LINE__);
	
	//calc gearing
	double gearing = 0.;
	for (unsigned int j = 0; j < x.size(); ++j)
	{
		gearing += mParam[j];
	}

	if (gearing == 0.)
		throw LACoreInvalidData("BSOption coefficient error",__FILE__,__LINE__);

	AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[calcIndex]);
	unsigned int i = 0; 
	double rate = 0.0;
	for (i = 0; i < x.size(); i++)
	{
		rate += mParam[i] * x[i] / gearing;
	}
//#ifdef ZEROFLOOR
//	rate = LAMath::max(rate, MIN_RATE);
//#endif
	param->F = rate;
	param->K = mParam[i] / gearing;
	AnalyticBKParam* slideParam(dynamic_cast<AnalyticBKParam*>(param->clone()));
	getForwardShiftParam(slideParam);

	double ret(0.);
	if (slideParam->F > MIN_RATE && slideParam->K > MIN_RATE && !mFixedPayOffs[calcIndex])
	{
		ret = mBSAnalyticMethod[calcIndex]->calc(*(slideParam)) * LAMath::abs(gearing);
		ret += calcConvexityAdjustment(*slideParam, ret / LAMath::abs(gearing)) * LAMath::abs(gearing);
	}
	else
	{
		ret = mBSPayoffMethod[calcIndex]->calc(*(slideParam)) * LAMath::abs(gearing);
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
			throw LACoreInvalidData("Option type error",__FILE__,__LINE__);

		ret = ret - sgn * slideParam->Nu * (slideParam->F - slideParam->K) * LAMath::abs(gearing);
	}
	
	delete slideParam;

	return ret;
}

LAStringVector
LABlackScholesCapletOption::getOptionTypeVector()
{
	LAStringVector ret(1);
	ret[0] = LAString(CALL);
	mOptionType = ret;
	return ret;
}

LAStringVector 
LABlackScholesCapletOption::getBSComponentVector(LAString risktype) const
{
	LAStringVector ret(1);
	ret[0] = LAString(BK) + risktype + LAString(CALL);
	return ret;
}

LAStringVector 
LABlackScholesCapletOption::getBSPayoffComponentVector(LAString risktype) const
{
	LAStringVector ret(1);
	ret[0] = LAString(BKPAYOFF) + risktype + LAString(CALL);
	return ret;
}

std::vector<AnalyticParam* >
LABlackScholesCapletOption::getAnalyticParam(void)
{
		mBSInputParam.clear();
		mBSInputParam.push_back(new AnalyticBKParam());

		BoolVector tmp(1,false);
		mFixedPayOffs = tmp;

	
	return mBSInputParam;
}

void 
LABlackScholesCapletOption::setOptionTerm(double Te)
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
LABlackScholesCapletOption::setOptionActualTerm(double actT)
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
LABlackScholesCapletOption::setOptionNumeraire(void)
{
	//if size > 1 then collar or strangle ..etc
	for (unsigned int i = 0; i < mBSInputParam.size(); i++)
	{
		AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[i]);
		param->Nu = 1.0;
	}
}

void
LABlackScholesCapletOption::setOptionStrike(void)
{
	//if size > 1 then collar or strangle ..etc
	unsigned int coeffsize = mParam.size();
	{
		unsigned int paraSize = mBSInputParam.size();
		for (unsigned int i = 0; i < paraSize; i++)
		{
			if (coeffsize-1-i < 0 || mParam[0] == 0.)
				throw LACoreInvalidData("Coefficient Error",__FILE__,__LINE__);

			AnalyticBKParam* param = dynamic_cast<AnalyticBKParam*>(mBSInputParam[paraSize-1-i]);
			param->K = mParam[coeffsize-1-i] / mParam[0];
		}
		
	}
}

void
LABlackScholesCapletOption::setIsAfterMaturity(bool isaftermaturity)
{
	if (!isaftermaturity)
		return;

	for (unsigned int i = 0; i < mBSInputParam.size(); i++)
		mFixedPayOffs[i] = true;

}


