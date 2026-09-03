/*! @file
    @brief Source code of class to represent linear function

    This class derives from AQLFunctionBase

*/
//  2010, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackScholesBaseMethod.cpp
//
//  SYNOPSIS    :       LABlackScholesBaseMethod
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


#include "LABlackScholesBaseFunc.h"
#include "AQLBasic.h"
#include "AQLDist.h"
#include "AQLCoreComponentManager.h"

using namespace std;
//================ LABlackScholesBaseMethod ===================================
/*!
	@brief default constructor
*/
LABlackScholesBaseMethod::LABlackScholesBaseMethod() 
: AQLFunctionBase(), mBSAnalyticMethod(std::vector<LABlackScholesBase* >(0)), mBSInputParam(std::vector<AnalyticParam* >(0))
,mIsAfterMaturity(false),mForwardShiftValue(0.),mFixedPayOffs(BoolVector())
{

}

/*!
	@brief destructor
*/
LABlackScholesBaseMethod::~LABlackScholesBaseMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LABlackScholesBaseMethod::clone() const
{
    try 
	{
		return new LABlackScholesBaseMethod(*this);
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
LABlackScholesBaseMethod::isTypeOf(function_t id) const
{
	return (id == FN_BSBASEFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LABlackScholesBaseMethod::getType() const
{
	return FN_BSBASEFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LABlackScholesBaseMethod::operator()(const DoubleArray& x) const
{
	throw AQLCoreInvalidData("Not support now",__FILE__,__LINE__);
	return 0.0;
}

std::vector<LABlackScholesBase* > 
LABlackScholesBaseMethod::getAnalyticMethod(AQLString risktype)
{
	AQLStringVector bsstrvec = getBSComponentVector(risktype);

	if (bsstrvec.size() < 1)
		throw AQLCoreInvalidData("BSComponent Size error",__FILE__,__LINE__);

	std::map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, LABlackScholesBase*> ::iterator it;
	
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

std::vector<LABlackScholesBase* > 
LABlackScholesBaseMethod::getPayoffMethod(AQLString risktype)
{
	AQLStringVector bsstrvec = getBSPayoffComponentVector(risktype);

	if (bsstrvec.size() < 1)
		throw AQLCoreInvalidData("BSComponent Size error",__FILE__,__LINE__);

	std::map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, LABlackScholesBase*> ::iterator it;
	
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
LABlackScholesBaseMethod::setForwardShiftValue(double forwardshiftvalue)
{
	mForwardShiftValue = forwardshiftvalue;
}

void
LABlackScholesBaseMethod::getForwardShiftParam(AnalyticBKParam* param) const
{
	param->F += mForwardShiftValue;
	param->K += mForwardShiftValue;
}