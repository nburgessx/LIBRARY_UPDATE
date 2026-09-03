/*! @file
    @brief Source code of class to represent RangeCount function

    This class derives from AQLFunctionBase

*/
//   2007,AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesUnionFunc.cpp
//
//  SYNOPSIS    :       LARatesUnionFunc
//  DESCRIPTION :       Source code of class to represent RangeCount function
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


#include "LARatesUnionFunc.h"
#include <algorithm>

using namespace std;
//================ LARatesUnionFunc ===================================
/*!
	@brief default constructor
*/
LARatesUnionFunc::LARatesUnionFunc() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
LARatesUnionFunc::~LARatesUnionFunc() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LARatesUnionFunc::clone() const
{
    try 
	{
		return new LARatesUnionFunc(*this);
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
LARatesUnionFunc::isTypeOf(function_t id) const
{
	return (id == FN_UNIONFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesUnionFunc::getType() const
{
	return FN_UNIONFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LARatesUnionFunc::operator()(const DoubleArray& x) const
{
	if (mParam.size() != x.size() * 4 + 1)
	{
		AQLString msg = "parameter size must be index * 4 + 1:lower[1],upper[1],,,,inequalityflag[1],inequalityflag[1],...,union or intersection flag";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	double ret = -1.0;
	int N = (mParam.size() - 1) / 2;

	// if isUnion is false, the condition is intersection.
	bool isUnion = false;
	if (mParam.back() == 1.)
		isUnion = false;
	else if (mParam.back() == 0.)
		isUnion = true;
	else
		throw AQLCoreInvalidData("The last parameter must be 1 or 0!",__FILE__,__LINE__);
	
	if (isUnion)
	{
		ret = -1.;

		for (unsigned int i = 0; i < x.size(); ++i)
		{
			if (mParam[2*i] > mParam[2*i+1])
				throw AQLCoreInvalidData("wrong input lower > upper",__FILE__,__LINE__);

			if ((mParam[2*i+N] != 0. && mParam[2*i+N] != 1.) || (mParam[2*i+1+N] != 0. && mParam[2*i+1+N] != 1.))
				throw AQLCoreInvalidData("Equal include parameter must be 1 or 0!",__FILE__,__LINE__);
		}

		for (unsigned int i = 0; i < x.size(); ++i)
		{			
			if (mParam[2*i]< x[i] && x[i]< mParam[2*i+1])
			{	
				ret = 1.0;
				break;
			}

			if (mParam[2*i] == x[i])
			{
				if(mParam[2*i+N] ==1.0 && mParam[2*i+N+1] == 1.0 && x[i] <= mParam[2*i+1] ||
					mParam[2*i+N] ==1.0 && mParam[2*i+N+1] == 0.0 && x[i] < mParam[2*i+1])
				{	
					ret = 1.0;
					break;
				}
			}

			if (mParam[2*i+1] == x[i])
			{
				if(mParam[2*i+N+1] == 1.0 && mParam[2*i+N] == 1.0 && mParam[2*i] <= x[i] ||
					mParam[2*i+N+1] == 1.0 && mParam[2*i+N] == 0.0 && mParam[2*i] < x[i])
				{
					ret = 1.0;
					break;
				}
			}
		}
	}
	else
	{	
		ret = 1.;

		for (unsigned int i = 0; i < x.size(); ++i)
		{
			if (mParam[2*i] > mParam[2*i+1])
				throw AQLCoreInvalidData("wrong input lower > upper",__FILE__,__LINE__);

			if ((mParam[2*i+N] != 0. && mParam[2*i+N] != 1.) || (mParam[2*i+1+N] != 0. && mParam[2*i+1+N] != 1.))
				throw AQLCoreInvalidData("Equal include parameter must be 1 or 0!",__FILE__,__LINE__);
		}

		for (unsigned int i = 0; i < x.size(); ++i)
		{
			if (mParam[2*i] == mParam[2*i+1] && mParam[2*i+N] != mParam[2*i+N+1])
			{
				ret = -1.;
				break;
			}

			if (x[i] < mParam[2*i] || mParam[2*i+1] < x[i])
			{	
				ret = -1.;
				break;
			}

			if (mParam[2*i] == x[i])
			{
				if (mParam[2*i+N] == 0.)
				{	
					ret = -1.;
					break;
				}
			}

			if (mParam[2*i+1] == x[i])
			{
				if (mParam[2*i+N+1] == 0.)
				{
					ret = -1.;
					break;
				}
			}
		}
	}

	return ret;
}


