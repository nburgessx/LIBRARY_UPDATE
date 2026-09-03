/*! @file
    @brief Source code of class to represent RangeCount function

    This class derives from AQLFunctionBase

*/
//   2007,AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesUnionLinearFunc.cpp
//
//  SYNOPSIS    :       LARatesUnionLinearFunc
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


#include "LARatesUnionLinearFunc.h"
#include <algorithm>

using namespace std;
//================ LARatesUnionLinearFunc ===================================
/*!
	@brief default constructor
*/
LARatesUnionLinearFunc::LARatesUnionLinearFunc() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
LARatesUnionLinearFunc::~LARatesUnionLinearFunc() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LARatesUnionLinearFunc::clone() const
{
    try 
	{
		return new LARatesUnionLinearFunc(*this);
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
LARatesUnionLinearFunc::isTypeOf(function_t id) const
{
	return (id == FN_UNIONLINEARFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesUnionLinearFunc::getType() const
{
	return FN_UNIONLINEARFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LARatesUnionLinearFunc::operator()(const DoubleArray& x) const
{
	if (mParam.size()!= x.size() + 5)
	{
		AQLString msg = "parameter size must be index +5:lower,upper,coef[1],,,,constant,lflag,rflag";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	double ret = -1.0;
	
	double val = 0.0;
	int N = x.size();
	for(int i=0;i<N;++i)
		val += mParam[i+2] * x[i];
	val += mParam[N+2];

	if(mParam[0]>mParam[1])
			throw AQLCoreInvalidData("wrong input lower > upper",__FILE__,__LINE__);
		
	if((mParam[0]< val && val < mParam[1]))
	{	
		return 1.0;	
	}
	if(mParam[0]==val)
	{
		if(mParam[N+3]==1.0 && mParam[N+4] ==1.0 && val <=mParam[1] ||
			mParam[N+3]==1.0 && mParam[N+4] ==0.0 && val <mParam[1])
		{	
			return 1.0;
		}
	}
	if(mParam[1]==val)
	{
		if(mParam[N+4]==1.0 && mParam[N+3] ==1.0 && mParam[0] <= val ||
			mParam[N+4]==1.0 && mParam[N+3] ==0.0 && mParam[0] < val)
		{
			return 1.0;
		}
	}
	
	return ret;
}


