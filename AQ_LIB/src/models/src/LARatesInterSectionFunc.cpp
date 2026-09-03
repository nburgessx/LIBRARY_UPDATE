/*! @file
    @brief Source code of class to represent RangeCount function

    This class derives from LAFunctionBase

*/
//   2007,AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesInterSectionFunc.cpp
//
//  SYNOPSIS    :       LARatesInterSectionFunc
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


#include "LARatesInterSectionFunc.h"
#include <algorithm>

using namespace std;
//================ LARatesInterSectionFunc ===================================
/*!
	@brief default constructor
*/
LARatesInterSectionFunc::LARatesInterSectionFunc() 
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LARatesInterSectionFunc::~LARatesInterSectionFunc() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LARatesInterSectionFunc::clone() const
{
    try 
	{
		return new LARatesInterSectionFunc(*this);
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
LARatesInterSectionFunc::isTypeOf(function_t id) const
{
	return (id == FN_INTERSECTIONFUNC ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesInterSectionFunc::getType() const
{
	return FN_INTERSECTIONFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LARatesInterSectionFunc::operator()(const DoubleArray& x) const
{
	if (mParam.size()!= x.size()*4)
	{
		LAString msg = "parameter size must be index * 4:lower[1],upper[1],,,,inequalityflag[1],inequalityflag[1],...";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	double ret = 1.0;
	int N = mParam.size()/2;

	for( unsigned int i = 0; i<x.size();i++)
	{
		if(mParam[2*i]>mParam[2*i+1])
			throw LACoreInvalidData("wrong input lower > upper",__FILE__,__LINE__); 
		
		if(!(mParam[2*i]<=x[i] && x[i]<=mParam[2*i+1]))
		{	
			ret =-1.0;
			break;
		}
		if(mParam[2*i]==x[i])
		{
			if(mParam[2*i+N]==0.0)
			{	
				ret=-1.0;
				break;
			}
		}
		if(mParam[2*i+1]==x[i])
		{
			if(mParam[2*i+N+1]==0.0)
			{
				ret=-1.0;
				break;
			}
		}
	}

	return ret;
}


