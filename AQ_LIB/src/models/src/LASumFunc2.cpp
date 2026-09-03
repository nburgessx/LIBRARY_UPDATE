/*! @file
    @brief Source code of class to represent sum function

    This class derives from AQLFunctionBase

*/
//  2006, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LASumMethod2.cpp
//
//  SYNOPSIS    :       LASumMethod2
//  DESCRIPTION :       Source code of class to represent sum function
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


#include "LASumFunc2.h"
#include <algorithm>

using namespace std;
//================ LASumMethod2 ===================================
/*!
	@brief default constructor
*/
LASumMethod2::LASumMethod2() 
: AQLFunctionBase()
{

}

/*!
	@brief destructor
*/
LASumMethod2::~LASumMethod2() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*	
LASumMethod2::clone() const
{
    try 
	{
		return new LASumMethod2(*this);
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
LASumMethod2::isTypeOf(function_t id) const
{
	return (id == FN_SUM2 ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LASumMethod2::getType() const
{
	return FN_SUM2;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LASumMethod2::operator()(const DoubleArray& x) const
{
	double ret = 0.0;
	for (unsigned int i = 0; i < x.size()-1; i++)
		ret += x[i];
	return ret;
}

/*!
    @brief Return partial derivative value
	@param[in] x integral point
	@param[in] pos variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] difftype both side or one side
	@param[in] delta grid width for numerical method
    @return partial deribative value
*/

double
LASumMethod2::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (pos >= x.size())
	{
        throw AQLCoreInvalidData("pos is over x size", __FILE__, __LINE__);
	}
	
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
	else
	{
		return 1.0;
	}

}

/*!
    @brief Return second partial derivative value
	@param[in] x integral point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
	@param[in] calctype analytical or numerical integral
	@param[in] delta grid width for numerical method
    @return second partial deribative value
*/
double
LASumMethod2::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return AQLFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
	else
	{
		return 0;
	}
}

/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] pos variable location to implement derivative
    @return derivable or not
*/
/*
bool
LASumMethod2::isDifferentiable(const DoubleArray& x, unsigned int pos) 
const
{
	DoubleArray::const_iterator it = max_element(x.begin(), x.end());
	unsigned int i = static_cast<unsigned int>(it - x.begin());
	if (i != pos)
	{
		if (x.at(pos) < *it) return true;
		//error
		return false;
	}
		
	double v = *it;
	DoubleArray tmp = x;
	tmp.erase(max_element(tmp.begin(), tmp.end()));
	it = max_element(tmp.begin(), tmp.end());
	if (v == *it) return false;	
	return true;

}
*/
/*!
    @brief Return derivable or not
	@param[in] x point
	@param[in] posi one variable location to implement derivative
	@param[in] posj another variable location to implement derivative
    @return derivable or not
*/
/*
bool
LASumMethod2::isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) 
const
{
	return isDifferentiable(x, posi) && isDifferentiable(x, posj); 
}
*/
