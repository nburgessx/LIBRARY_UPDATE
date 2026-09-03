/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLFunctionBase

*/
//  2010, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLBlackScholesBaseMethod.cpp
//
//  SYNOPSIS    :       AQLBlackScholesBaseMethod
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


#include "AQLBlackScholesBaseFunc.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLCoreComponentManager.h"

using namespace std;
//================ AQLBlackScholesBaseMethod ===================================
/*!
	@brief default constructor
*/
AQLBlackScholesBaseMethod::AQLBlackScholesBaseMethod() 
: AQLFunctionBase(), mBSAnalyticMethod(std::vector<AQLBlackScholesBase* >(0)), mBSInputParam(std::vector<AnalyticParam* >(0))
,mIsAfterMaturity(false),mForwardShiftValue(0.),mFixedPayOffs(BoolVector())
{

}

/*!
	@brief destructor
*/
AQLBlackScholesBaseMethod::~AQLBlackScholesBaseMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLBlackScholesBaseMethod::clone() const
{
    try 
	{
		return new AQLBlackScholesBaseMethod(*this);
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
AQLBlackScholesBaseMethod::isTypeOf(function_t id) const
{
	return (id == FN_BSBASEFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLBlackScholesBaseMethod::getType() const
{
	return FN_BSBASEFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLBlackScholesBaseMethod::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("Not support now",__FILE__,__LINE__);
	return 0.0;
}

std::vector<AQLBlackScholesBase* > 
AQLBlackScholesBaseMethod::getAnalyticMethod(AQLString risktype)
{
	AQLStringVector bsstrvec = getBSComponentVector(risktype);

	if (bsstrvec.size() < 1)
		throw AQLCoreInvalidData("BSComponent Size error",__FILE__,__LINE__);

	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it;
	
	mBSAnalyticMethod.resize(bsstrvec.size());
	for (unsigned int i = 0; i < bsstrvec.size(); i++)
	{
		 it = var.find(bsstrvec[i]);
		 if(it==var.end())
			throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);

		 mBSAnalyticMethod[i] = it->second;
	}
	return mBSAnalyticMethod;

}

std::vector<AQLBlackScholesBase* > 
AQLBlackScholesBaseMethod::getPayoffMethod(AQLString risktype)
{
	AQLStringVector bsstrvec = getBSPayoffComponentVector(risktype);

	if (bsstrvec.size() < 1)
		throw AQLCoreInvalidData("BSComponent Size error",__FILE__,__LINE__);

	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it;
	
	mBSPayoffMethod.resize(bsstrvec.size());
	for (unsigned int i = 0; i < bsstrvec.size(); i++)
	{
		 it = var.find(bsstrvec[i]);
		 if(it==var.end())
			throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);

		 mBSPayoffMethod[i] = it->second;
	}
	return mBSPayoffMethod;

}

void 
AQLBlackScholesBaseMethod::setForwardShiftValue(double forwardshiftvalue)
{
	mForwardShiftValue = forwardshiftvalue;
}

void
AQLBlackScholesBaseMethod::getForwardShiftParam(AnalyticBKParam* param) const
{
	param->F += mForwardShiftValue;
	param->K += mForwardShiftValue;
}