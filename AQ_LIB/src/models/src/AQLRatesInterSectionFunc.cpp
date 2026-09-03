/*! @file
    @brief Source code of class to represent RangeCount function

    This class derives from AQLFunctionBase

*/
//   2007,AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRatesInterSectionFunc.cpp
//
//  SYNOPSIS    :       AQLRatesInterSectionFunc
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


#include "AQLRatesInterSectionFunc.h"
#include <algorithm>

using namespace std;
//================ AQLRatesInterSectionFunc ===================================
/*!
	@brief default constructor
*/
AQLRatesInterSectionFunc::AQLRatesInterSectionFunc() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
AQLRatesInterSectionFunc::~AQLRatesInterSectionFunc() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
AQLRatesInterSectionFunc::clone() const
{
    try 
	{
		return new AQLRatesInterSectionFunc(*this);
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
AQLRatesInterSectionFunc::isTypeOf(function_t id) const
{
	return (id == FN_INTERSECTIONFUNC ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesInterSectionFunc::getType() const
{
	return FN_INTERSECTIONFUNC;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
AQLRatesInterSectionFunc::operator()(const DoubleArray& x) const
{
	if (mParam.size()!= x.size()*4)
	{
		AQLString msg = "parameter size must be index * 4:lower[1],upper[1],,,,inequalityflag[1],inequalityflag[1],...";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	double ret = 1.0;
	int N = mParam.size()/2;

	for( unsigned int i = 0; i<x.size();i++)
	{
		if(mParam[2*i]>mParam[2*i+1])
			throw AQLCoreInvalidData("wrong input lower > upper",__FILE__,__LINE__); 
		
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


