/*! @file
    @brief Source code of class to represent sum function

    This class derives from LAFunctionBase

*/
//  2006, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LASumMethod.cpp
//
//  SYNOPSIS    :       LASumMethod
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


#include "LASumFunc.h"
#include <algorithm>

using namespace std;
//================ LASumMethod ===================================
/*!
	@brief default constructor
*/
LASumMethod::LASumMethod() 
: LAFunctionBase()
{

}

/*!
	@brief destructor
*/
LASumMethod::~LASumMethod() 
{
}

/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*	
LASumMethod::clone() const
{
    try 
	{
		return new LASumMethod(*this);
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
LASumMethod::isTypeOf(function_t id) const
{
	return (id == FN_SUM ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LASumMethod::getType() const
{
	return FN_SUM;
}

/*!
    @brief Return function value
	@param[in] x point
    @return function value
*/
double
LASumMethod::operator()(const DoubleArray& x) const
{
	double ret = 0.0;
	for (unsigned int i = 0; i < x.size(); i++)
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
LASumMethod::partialDerivative(const DoubleArray& x, unsigned int pos,
								CALC_TYPE calctype, DIFF_TYPE difftype, double delta) const
{
	if (pos >= x.size())
	{
        throw LACoreInvalidData("pos is over x size", __FILE__, __LINE__);
	}
	
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative(x, pos, calctype, difftype, delta);
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
LASumMethod::partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype, double delta) const
{
	if (calctype == NUMERICAL)
		return LAFunctionBase::partialDerivative2(x, 0, 0, calctype, delta);
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
LASumMethod::isDifferentiable(const DoubleArray& x, unsigned int pos) 
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
LASumMethod::isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) 
const
{
	return isDifferentiable(x, posi) && isDifferentiable(x, posj); 
}
*/
