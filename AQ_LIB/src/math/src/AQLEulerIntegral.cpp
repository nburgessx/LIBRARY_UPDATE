/*! @file
    @brief
*/



////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MMNaturalIntegral.cpp
//
//  SYNOPSIS    :       MMNaturalIntegral
//  DESCRIPTION :       
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLEulerIntegral.h"
#include <limits>


//================ MMNaturalIntegral ===================================
/*!
	@brief Constructor
	@param[in] numberOfPoints divident number in integral region
*/
AQLEulerIntegral::AQLEulerIntegral(unsigned short numberOfPoints)
:
AQL1DIntegral(),
mNumberOfPoints(numberOfPoints)
{
}

/*!
	@brief Destructor
*/
AQLEulerIntegral::~AQLEulerIntegral() 
{
}

/*!
    @brief Check function for this class type
    @param[in] id class type to check
    @return true or false
*/
bool
AQLEulerIntegral::isTypeOf(function_t id) const
{
    return (id == FN_EULERINTEGRAL ? true : AQLIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
AQLEulerIntegral::getType() const
{
    return FN_EULERINTEGRAL;  
}


/*!
    @brief  Integration by Natural Integral methods.

    @param[in] f target function
	@param[in] xl integral lower bound
	@param[in] xu integral upper bound

	@return integral result
*/
double
AQLEulerIntegral::integrate(const AQLFunctionBase& f,double xl,double xu) const
{
	if(xl > xu) return -integrate(f, xu, xl);

	DoubleArray xx(mNumberOfPoints);
	DoubleArray ww(mNumberOfPoints);
	
	get(xx, ww, xl, xu);
	
	DoubleArray method(mNumberOfPoints);
    for (unsigned short i = 0; i < mNumberOfPoints; i++)
	{
		method[i] = f(xx[i]);
	}
    double ret = 0.0;
	for(unsigned int i = 0; i < method.size(); i++)
		ret += method[i] * ww[i];
	return ret;
}

void
AQLEulerIntegral::get(DoubleArray& xx, DoubleArray& ww, double a, double b) const
{
	double dx = (b - a) / mNumberOfPoints;
	
	for(size_t i = 0; i < mNumberOfPoints; ++i)
	{
		if(i == 0)
		{
			ww[0] = dx;
			xx[0] = a + numeric_limits<double>::epsilon();
		}
		else
		{
			ww[i] = dx;
			xx[i] = a + i * dx;
		}
	}
}

//
//--------------------------------------------------------------------------------
//

/*!
	@brief Constructor
	@param[in] numberOfPoints divident number in integral region
*/
AQLPCIntegral::AQLPCIntegral(const vector<double>& x_)
:
AQL1DIntegral(),
x(x_),
mNumberOfPoints(x.size() - 1)
{
}

/*!
	@brief Destructor
*/
AQLPCIntegral::~AQLPCIntegral() 
{
}

/*!
    @brief Check function for this class type
    @param[in] id class type to check
    @return true or false
*/
bool
AQLPCIntegral::isTypeOf(function_t id) const
{
    return (id == FN_EULERINTEGRAL ? true : AQLIntegralBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return class type
*/
function_t
AQLPCIntegral::getType() const
{
    return FN_EULERINTEGRAL;  
}


/*!
    @brief  Integration by Natural Integral methods.

    @param[in] f target function
	@param[in] xl integral lower bound
	@param[in] xu integral upper bound

	@return integral result
*/
double
AQLPCIntegral::integrate(const AQLFunctionBase& f,double xl,double xu) const
{
	if(xl > xu) return -integrate(f, xu, xl);

	DoubleArray xx(mNumberOfPoints);
	DoubleArray ww(mNumberOfPoints);
	
	get(xx, ww, xl, xu);
	
	DoubleArray method(mNumberOfPoints);
    for (unsigned short i = 0; i < mNumberOfPoints; i++)
	{
		method[i] = f(xx[i]);
	}
    double ret = 0.0;
	for(unsigned int i = 0; i < method.size(); i++)
		ret += method[i] * ww[i];
	return ret;
}

void
AQLPCIntegral::get(DoubleArray& xx, DoubleArray& ww, double a, double b) const
{
	for(size_t i = 0; i < mNumberOfPoints; ++i)
	{
		ww[i] = x[i+1] - x[i];
		xx[i] = x[i];
	}
}